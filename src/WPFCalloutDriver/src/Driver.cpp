#include "driver.hpp"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

WDFDEVICE g_wdfDevice;
PDEVICE_OBJECT g_deviceObject;
HANDLE          g_engineHandle;       // handle for the open session to the filter engine

_Use_decl_annotations_
void ClassifyFn(
    const FWPS_INCOMING_VALUES0* inFixedValues,
    const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    void* layerData,
    const void* classifyContext,
    const FWPS_FILTER3* filter,
    UINT64 flowContext,
    FWPS_CLASSIFY_OUT0* classifyOut
)
{
    UNREFERENCED_PARAMETER(inFixedValues);
    UNREFERENCED_PARAMETER(inMetaValues);
    UNREFERENCED_PARAMETER(layerData);
    UNREFERENCED_PARAMETER(classifyContext);
    UNREFERENCED_PARAMETER(filter);
    UNREFERENCED_PARAMETER(flowContext);

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ClassifyFn()\n"));
    classifyOut->actionType = FWP_ACTION_CONTINUE;
}

_Use_decl_annotations_
NTSTATUS NotifyFn(
    FWPS_CALLOUT_NOTIFY_TYPE notifyType, 
    const GUID* filterKey, 
    FWPS_FILTER3* filter
)
{
    UNREFERENCED_PARAMETER(notifyType);
    UNREFERENCED_PARAMETER(filterKey);
    UNREFERENCED_PARAMETER(filter);

    return STATUS_SUCCESS;
}

NTSTATUS RegisterCallouts(
    GUID  calloutKey,
    GUID* providerKey,
    GUID  applicableLayer,
    FWPS_CALLOUT_CLASSIFY_FN3 classifyCallout,
    FWPS_CALLOUT_NOTIFY_FN3 notifyCallout
)
{
    //auto x1 = FWPM_LAYER_OUTBOUND_TRANSPORT_V4;
    //auto x2 = FWPM_LAYER_OUTBOUND_IPPACKET_V4;

    if (!g_deviceObject)
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "RegisterCallouts(): g_deviceObject is nullptr\n"));
        return STATUS_INVALID_HANDLE;
    }

    UINT32 calloutId = 0;
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/ns-fwpsk-fwps_callout2_
    // FWPS_CALLOUT3 doesn't appear to be documented, only 0-2 are
    FWPS_CALLOUT3 sCallout
    {
        .calloutKey = calloutKey,
        .classifyFn = classifyCallout,
        .notifyFn = notifyCallout
    };

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/nf-fwpsk-fwpscalloutregister0
    // This should actually be FwpsCalloutRegister3, but it's not documented
    NTSTATUS status = FwpsCalloutRegister3(
        g_deviceObject,
        &sCallout,
        &calloutId
    );
    if (NT_ERROR(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FwpsCalloutRegister3() failed %lu\n", status));
        return status;
    }

    wchar_t x[] = L"ToyDriverCallout";
    FWPM_CALLOUT0 mCallout = {
        .calloutKey = calloutKey,
        .displayData = {.name = x },
        .providerKey = providerKey,
        .applicableLayer = applicableLayer,
    };
    status = FwpmCalloutAdd0(
        g_engineHandle,
        &mCallout,
        nullptr,         // default security desc
        &calloutId
    );  // same as about calloutid
    if (NT_ERROR(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FwpmCalloutAdd0() failed %lu\n", status));
        return status;
    }
    //LogInfo("  Added callout: %llu", calloutId);
    return status;
}

NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    FWPM_SESSION0   session = { 0 };
    FWPM_PROVIDER0  provider = { 0 };
    wchar_t providerName[] = L"provider name";
    bool addedProvider = false;

    // See https://docs.microsoft.com/en-us/windows-hardware/drivers/network/specifying-an-unload-function

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdriver/nf-wdfdriver-wdf_driver_config_init
    WDF_DRIVER_CONFIG config;
    WDF_DRIVER_CONFIG_INIT(&config, nullptr);

    // Indicate that this is a non-PNP driver
    config.DriverInitFlags |= WdfDriverInitNonPnpDriver;

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdriver/nc-wdfdriver-evt_wdf_driver_unload
    // WDF Drivers use this, otherwise, WDM drivers use DriverObject->DriverUnload = DriverUnload (https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nc-wdm-driver_unload)
    config.EvtDriverUnload = DriverUnload;

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfobject/nf-wdfobject-wdf_object_attributes_init
    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

    WDFDRIVER driver = nullptr;
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdriver/nf-wdfdriver-wdfdrivercreate
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDriverCreate()\n"));
    NTSTATUS status = WdfDriverCreate(
        DriverObject,
        RegistryPath,
        &attributes,
        &config,
        &driver
    );
    if (NT_ERROR(status)) 
        return status;

    // Allocate a device initialization structure
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfcontrol/nf-wdfcontrol-wdfcontroldeviceinitallocate
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfControlDeviceInitAllocate()\n"));
    PWDFDEVICE_INIT deviceInit = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_KERNEL_ONLY);
    if (!deviceInit)
        return STATUS_FAILED_DRIVER_ENTRY;

    // Set the device characteristics
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdeviceinitsetcharacteristics
    // This is taken from https://docs.microsoft.com/en-us/windows-hardware/drivers/network/creating-a-device-object
    // It does not work, WdfDeviceInitSetCharacteristics() does not appear to have a characteristic for FILE_DEVICE_SECURE_OPEN
    // See Characteristics for  https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_device_object
    /*KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceInitSetCharacteristics()\n"));
    WdfDeviceInitSetCharacteristics(
        deviceInit,
        FILE_DEVICE_SECURE_OPEN,
        false
    );*/

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceInitSetDeviceType()\n"));
    WdfDeviceInitSetDeviceType(deviceInit, FILE_DEVICE_NETWORK);

    // Create a framework device object
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdevicecreate
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceCreate()\n"));
    status = WdfDeviceCreate(&deviceInit, WDF_NO_OBJECT_ATTRIBUTES, &g_wdfDevice);
    if (NT_ERROR(status))
        goto ERRORCLEANUP;

    // Initialization of the framework device object is complete
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfcontrol/nf-wdfcontrol-wdfcontrolfinishinitializing
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfControlFinishInitializing()\n"));
    WdfControlFinishInitializing(g_wdfDevice);

    // Get the associated WDM device object
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdevicewdmgetdeviceobject
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceWdmGetDeviceObject()\n"));
    g_deviceObject = WdfDeviceWdmGetDeviceObject(g_wdfDevice);
    if (!g_deviceObject)
        goto ERRORCLEANUP;

    // Open handle to the filtering engine
    status = FwpmEngineOpen0(
        nullptr,                   // The filter engine on the local system
        RPC_C_AUTHN_DEFAULT,    // Use the Windows authentication service
        nullptr,                   // Use the calling thread&#39;s credentials
        &session,               // There are session-specific parameters
        &g_engineHandle     // Pointer to a variable to receive the handle
    );
    if (NT_ERROR(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FwpmEngineOpen0() failed\n"));
        goto ERRORCLEANUP;
    }

    // Add the provider
    provider.displayData.name = providerName;
    provider.providerKey = WFP_PROVIDER_GUID;
    status = FwpmProviderAdd0(
        g_engineHandle,
        (const FWPM_PROVIDER0*)&provider,
        nullptr
    ); 
    if (NT_ERROR(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FwpmProviderAdd0() failed %lu\n", status));
        return status;
    }
    addedProvider = true;

    // Register our callouts
    status = RegisterCallouts(
        WFP_TEST_CALLOUT,
        (GUID*)&WFP_PROVIDER_GUID,
        FWPM_LAYER_INBOUND_IPPACKET_V4,
        ClassifyFn, 
        NotifyFn
    );
    if (NT_ERROR(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "RegisterCallouts() failed\n"));
        goto ERRORCLEANUP;
    }

    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmsublayeradd0
    //FwpmSubLayerAdd0

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/nf-fwpsk-fwpscalloutregister0
    //FwpsCalloutRegister0

    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmfilteradd0
    //FwpmFilterAdd0

    return status;

ERRORCLEANUP:
    if (addedProvider)
        FwpmProviderDeleteByKey0(g_engineHandle, &WFP_PROVIDER_GUID);
    if (g_engineHandle)
        FwpmEngineClose0(g_engineHandle);
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdeviceinitfree
    if (deviceInit)
        WdfDeviceInitFree(deviceInit);

    return status;
}

//void DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
void DriverUnload(_In_ WDFDRIVER DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
}
