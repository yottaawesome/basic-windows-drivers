#include "driver.hpp"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

WDFDEVICE wdfDevice;

/*++
Routine Description:
    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver, such as EvtDevice and DriverUnload.

Parameters Description:

    DriverObject - represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory.

    RegistryPath - represents the driver specific path in the Registry.
    The function driver can use the path to store driver related data between
    reboots. The path does not store hardware instance specific data.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    // See https://docs.microsoft.com/en-us/windows-hardware/drivers/network/specifying-an-unload-function

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfobject/nf-wdfobject-wdf_object_attributes_init
    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdriver/nf-wdfdriver-wdf_driver_config_init
    WDF_DRIVER_CONFIG config;
    WDF_DRIVER_CONFIG_INIT(&config, nullptr);

    // Indicate that this is a non-PNP driver
    config.DriverInitFlags |= WdfDriverInitNonPnpDriver;

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdriver/nc-wdfdriver-evt_wdf_driver_unload
    // WDF Drivers use this, otherwise, WDM drivers use DriverObject->DriverUnload = DriverUnload (https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nc-wdm-driver_unload)
    config.EvtDriverUnload = DriverUnload;

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
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceInitSetCharacteristics()\n"));
    WdfDeviceInitSetCharacteristics(
        deviceInit,
        FILE_DEVICE_SECURE_OPEN,
        false
    );

    // Create a framework device object
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdevicecreate
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceCreate()\n"));
    status = WdfDeviceCreate(&deviceInit, WDF_NO_OBJECT_ATTRIBUTES, &wdfDevice);
    if (NT_ERROR(status))
        goto ERRORCLEANUP;

    // Initialization of the framework device object is complete
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfcontrol/nf-wdfcontrol-wdfcontrolfinishinitializing
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfControlFinishInitializing()\n"));
    WdfControlFinishInitializing(wdfDevice);

    // Get the associated WDM device object
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdevicewdmgetdeviceobject
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceWdmGetDeviceObject()\n"));
    PDEVICE_OBJECT deviceObject = WdfDeviceWdmGetDeviceObject(wdfDevice);
    if (!deviceObject)
        goto ERRORCLEANUP;

    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmsublayeradd0
    //FwpmSubLayerAdd0

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/nf-fwpsk-fwpscalloutregister0
    //FwpsCalloutRegister0

    // https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmfilteradd0
    //FwpmFilterAdd0

    return status;

ERRORCLEANUP:
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
