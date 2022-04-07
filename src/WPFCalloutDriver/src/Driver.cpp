#include "driver.hpp"
#include "Globals.hpp"
#include "Identifiers.hpp"
#include "Callouts.hpp"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

extern "C" PULONG InitSafeBootMode;

WDFDEVICE ToyDriver::Globals::WDFDriverDevice = { 0 };
PDEVICE_OBJECT ToyDriver::Globals::DriverDeviceObject = nullptr;

_Use_decl_annotations_
NTSTATUS DriverEntry(
    PDRIVER_OBJECT  DriverObject,
    PUNICODE_STRING RegistryPath
)
{
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/determining-whether-the-operating-system-is-running-in-safe-mode
    if (*InitSafeBootMode > 0)
        return STATUS_NOT_SAFE_MODE_DRIVER;

    NTSTATUS status = STATUS_FAILED_DRIVER_ENTRY;
    PWDFDEVICE_INIT deviceInit = nullptr;
    do
    {
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
        if (status = WdfDeviceCreate(&deviceInit, WDF_NO_OBJECT_ATTRIBUTES, &ToyDriver::Globals::WDFDriverDevice); NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceCreate() failed %lu\n", status));
            break;
        }

        // Initialization of the framework device object is complete
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfcontrol/nf-wdfcontrol-wdfcontrolfinishinitializing
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfControlFinishInitializing()\n"));
        WdfControlFinishInitializing(ToyDriver::Globals::WDFDriverDevice);

        // Get the associated WDM device object
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdevicewdmgetdeviceobject
        if (ToyDriver::Globals::DriverDeviceObject = WdfDeviceWdmGetDeviceObject(ToyDriver::Globals::WDFDriverDevice); 
            !ToyDriver::Globals::DriverDeviceObject)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceWdmGetDeviceObject() failed %lu\n", status));
            break;
        }

        // Register our callouts
        if(status = ToyDriver::Callouts::RegisterAllCallouts(); NT_ERROR(status))
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

// Legacy driver signature = void DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
_Use_decl_annotations_
void DriverUnload(WDFDRIVER DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    ToyDriver::Callouts::UnregisterAllCallouts();
}
