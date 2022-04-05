#include "driver.hpp"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

extern "C" PULONG InitSafeBootMode;

WDFDEVICE g_wdfDevice;
PDEVICE_OBJECT g_deviceObject;

void LogFilter(const FWPS_FILTER3* filter)
{
    if (!filter)
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "filter is null\n"));
        return;
    }
    if (!filter->providerContext)
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "providerContext is null\n"));
        return;
    }
    if (!filter->providerContext->dataBuffer)
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "dataBuffer is null\n"));
        return;
    }
    if (!filter->providerContext->dataBuffer->size)
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "dataBuffer is empty\n"));
        return;
    }

    // filter->providerContext->providerData is always null, not matter what I pass for the
    // providerData at the filter, callout, or provider object creation stages. The MSDN
    // docs don't really provide any useful information as to why, and the only tangible
    // sources online refer only to filter->providerContext->dataBuffer instead.
}

DWORD GetProcessId(const FWPS_FILTER3* filter)
{
    if (!filter) 
        return 0;
    if (!filter->providerContext) 
        return 0;
    if (!filter->providerContext->dataBuffer) 
        return 0;
    if (!filter->providerContext->dataBuffer->size) 
        return 0;

    if (filter->providerContext->dataBuffer->size != sizeof(DWORD))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "GetProcessId(): data size mismatch\n"));
        return 0;
    }

    return *reinterpret_cast<DWORD*>(filter->providerContext->dataBuffer->data);
}

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
    UNREFERENCED_PARAMETER(inMetaValues);
    UNREFERENCED_PARAMETER(layerData);
    UNREFERENCED_PARAMETER(classifyContext);
    UNREFERENCED_PARAMETER(flowContext);

    // We only inspect traffic
    classifyOut->actionType = FWP_ACTION_CONTINUE;

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/ns-fwpsk-fwps_incoming_metadata_values0_
    // Not available at the IPV* layers: inMetaValues->packetDirection == FWP_DIRECTION_INBOUND
    switch (inFixedValues->layerId)
    {
        case FWPS_LAYER_INBOUND_IPPACKET_V4:
            //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ClassifyFn() matched process --> inbound IPv4 packet\n"));
            break;

        case FWPS_LAYER_OUTBOUND_IPPACKET_V4:
        {
            // https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/8c923f6b-ce7d-4246-a919-2424d6e1991f/process-id-from-fwpmlayeroutboundippacketv4-layer
            // Note that processId is not available at IPV* layers: https://docs.microsoft.com/en-us/windows-hardware/drivers/network/metadata-fields-at-each-filtering-layer
            // inMetaValues->currentMetadataValues & FWPS_METADATA_FIELD_PROCESS_ID)
            const size_t runningProcessId = reinterpret_cast<size_t>(PsGetCurrentProcessId());
            const DWORD processId = GetProcessId(filter);
            if (!processId || !runningProcessId || processId != runningProcessId)
                return;

            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ClassifyFn() matched process --> outbound IPv4 packet\n"));
            break;
        }
            

        case FWPS_LAYER_INBOUND_TRANSPORT_V4:
            //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ClassifyFn() matched process --> inbound TCP\n"));
            break;

        case FWPS_LAYER_OUTBOUND_TRANSPORT_V4:
            //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ClassifyFn() matched process --> outbound TCP\n"));
            break;

        default:
            break;
    }
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
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "NotifyFn()\n"));
    return STATUS_SUCCESS;
}

NTSTATUS RegisterCallout(
    GUID  calloutKey,
    FWPS_CALLOUT_CLASSIFY_FN3 classifyCallout,
    FWPS_CALLOUT_NOTIFY_FN3 notifyCallout
)
{
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

    return status;
}

NTSTATUS RegisterCallouts()
{
    NTSTATUS status = RegisterCallout(
        WFP_OUTBOUND_IPV4_CALLOUT_GUID,
        ClassifyFn,
        NotifyFn
    );
    if (NT_ERROR(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering WFP_OUTBOUND_IPV4_CALLOUT_GUID failed %lu\n", status));
        return status;
    }

    status = RegisterCallout(
        WFP_INBOUND_IPV4_CALLOUT_GUID,
        ClassifyFn,
        NotifyFn
    );
    if (NT_ERROR(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering WFP_INBOUND_IPV4_CALLOUT_GUID failed %lu\n", status));
        return status;
    }

    status = RegisterCallout(
        WFP_OUTBOUND_TCP_GUID,
        ClassifyFn,
        NotifyFn
    );
    if (NT_ERROR(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering WFP_OUTBOUND_TCP_GUID failed %lu\n", status));
        return status;
    }

    return status;
}

NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/determining-whether-the-operating-system-is-running-in-safe-mode
    if (*InitSafeBootMode > 0)
        return STATUS_NOT_SAFE_MODE_DRIVER;

    NTSTATUS status = STATUS_FAILED_DRIVER_ENTRY;
    PWDFDEVICE_INIT deviceInit = nullptr;
    do
    {
        FWPM_SESSION0   session = { 0 };
        FWPM_PROVIDER0  provider = { 0 };

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
        status = WdfDriverCreate(
            DriverObject,
            RegistryPath,
            &attributes,
            &config,
            &driver
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDriverCreate() failed %lu\n", status));
            return status;
        }

        // Allocate a device initialization structure
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfcontrol/nf-wdfcontrol-wdfcontroldeviceinitallocate
        if (deviceInit = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_KERNEL_ONLY); !deviceInit)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfControlDeviceInitAllocate() failed %lu\n", status));
            return STATUS_FAILED_DRIVER_ENTRY;
        }

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
        if (status = WdfDeviceCreate(&deviceInit, WDF_NO_OBJECT_ATTRIBUTES, &g_wdfDevice); NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceCreate() failed %lu\n", status));
            break;
        }

        // Initialization of the framework device object is complete
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfcontrol/nf-wdfcontrol-wdfcontrolfinishinitializing
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfControlFinishInitializing()\n"));
        WdfControlFinishInitializing(g_wdfDevice);

        // Get the associated WDM device object
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdevicewdmgetdeviceobject
        if (g_deviceObject = WdfDeviceWdmGetDeviceObject(g_wdfDevice); !g_deviceObject)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceWdmGetDeviceObject() failed %lu\n", status));
            break;
        }

        // Register our callouts
        if(status = RegisterCallouts(); NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "RegisterCallouts() failed %lu\n", status));
            break;
        }

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WFP driver initialised successfully\n"));
        return status;

    } while (false);

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdeviceinitfree
    if (deviceInit)
        WdfDeviceInitFree(deviceInit);

    return status;
}

//void DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
void DriverUnload(_In_ WDFDRIVER DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    if (const NTSTATUS status = FwpsCalloutUnregisterByKey0(&WFP_OUTBOUND_IPV4_CALLOUT_GUID); status == STATUS_SUCCESS)
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WFP driver shutdown successfully\n"));
    else
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WFP failed driver shutdown %lu\n", status));
}
