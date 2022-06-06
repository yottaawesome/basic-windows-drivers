#include <ntddk.h>
#include <wdf.h>
#include <Wdmsec.h>
#include "IO.hpp"

WDFDEVICE WDFDriverDevice;
PDEVICE_OBJECT DriverDeviceObject;
extern "C" PULONG InitSafeBootMode; // Set by Windows at runtime

void DriverUnload(WDFDRIVER driverObject)
{
    UNREFERENCED_PARAMETER(driverObject);
}

extern "C" NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT  driverObject,
    _In_ PUNICODE_STRING registryPath
)
{
    UNREFERENCED_PARAMETER(registryPath);

    // https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/determining-whether-the-operating-system-is-running-in-safe-mode
    if (*InitSafeBootMode > 0)
    {
        return STATUS_NOT_SAFE_MODE_DRIVER;
    }

    NTSTATUS status = STATUS_FAILED_DRIVER_ENTRY;
    PWDFDEVICE_INIT deviceInit = nullptr;
    DriverDeviceObject = nullptr;
    WDFDriverDevice = { 0 };
    do
    {
        WDF_DRIVER_CONFIG config;
        WDF_DRIVER_CONFIG_INIT(&config, nullptr);
        config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
        config.EvtDriverUnload = DriverUnload;
        config.EvtDriverDeviceAdd = EvtWdfDriverDeviceAdd;
        WDF_OBJECT_ATTRIBUTES attributes;
        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        WDFDRIVER driver = nullptr;
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdriver/nf-wdfdriver-wdfdrivercreate
        status = WdfDriverCreate(
            driverObject,
            registryPath,
            &attributes,
            &config,
            &driver
        );
        if (NT_ERROR(status))
        {
            // TODO: figure out a better way of logging, as this macro is annoying
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDriverCreate() failed %lu\n", status));
            return status;
        }
        
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfcontrol/nf-wdfcontrol-wdfcontroldeviceinitallocate
        deviceInit = WdfControlDeviceInitAllocate(driver, &SDDL_DEVOBJ_SYS_ALL_ADM_ALL);
        if (!deviceInit)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfControlDeviceInitAllocate() failed %lu\n", status));
            return STATUS_FAILED_DRIVER_ENTRY;
        }

        WDF_FILEOBJECT_CONFIG fileObject;
        // Can also specify close and cleanup callbacks
        WDF_FILEOBJECT_CONFIG_INIT(&fileObject, EvtWdfDeviceFileCreate, nullptr, nullptr);
        WdfDeviceInitSetFileObjectConfig(
            deviceInit,
            &fileObject,
            &attributes
        );

        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdeviceinitsetdevicetype
        // Note that the device must match the CTR_CODE device (first argument)
        WdfDeviceInitSetDeviceType(deviceInit, FILE_DEVICE_UNKNOWN);
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdeviceinitsetiotype
        WdfDeviceInitSetIoType(deviceInit, WdfDeviceIoBuffered);

        UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\BarebonesKMDF");
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdeviceinitassignname
        status = WdfDeviceInitAssignName(deviceInit, &devName);
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceInitAssignName() failed %lu\n", status));
            break;
        }

        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdevicecreate
        status = WdfDeviceCreate(&deviceInit, WDF_NO_OBJECT_ATTRIBUTES, &WDFDriverDevice);
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceCreate() failed %lu\n", status));
            break;
        }

        // Create and I/O queue to respond to user application requests.
        WDF_IO_QUEUE_CONFIG ioQueueConfig;
        WDFQUEUE hQueue;
        WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
            &ioQueueConfig,
            WdfIoQueueDispatchSequential
        );
        // A queue is required for handling I/O requests.
        ioQueueConfig.EvtIoDefault = EvtWdfIoQueueIoDefault;
        ioQueueConfig.EvtIoRead = EvtWdfIoQueueRead;
        ioQueueConfig.EvtIoWrite = EvtWdfIoQueueWrite;
        ioQueueConfig.EvtIoDeviceControl = EvtIoDeviceControl;
        status = WdfIoQueueCreate(
            WDFDriverDevice,
            &ioQueueConfig,
            WDF_NO_OBJECT_ATTRIBUTES,
            &hQueue
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfIoQueueCreate() failed %lu\n", status));
            break;
        }

        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfcontrol/nf-wdfcontrol-wdfcontrolfinishinitializing
        WdfControlFinishInitializing(WDFDriverDevice);

        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdevicewdmgetdeviceobject
        DriverDeviceObject = WdfDeviceWdmGetDeviceObject(WDFDriverDevice);
        if (!DriverDeviceObject)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceWdmGetDeviceObject() failed %lu\n", status));
            break;
        }

        // Create a symbolic link to allow applications to interact with the driver
        UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\BarebonesKMDF");
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdevice/nf-wdfdevice-wdfdevicecreatesymboliclink
        status = WdfDeviceCreateSymbolicLink(WDFDriverDevice, &symLink);
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "WdfDeviceCreateSymbolicLink() failed %lu\n", status));
            break;
        }

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Barebones WDF (KMDF) driver initialised successfully.\n"));

        return STATUS_SUCCESS;

    } while (false);

    return status;
}