#pragma warning(disable : 4471) // causes a warning in C++
#include <ntddk.h>
#include <wdf.h>

// Based on https://docs.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-a-very-small-kmdf--driver

extern "C" DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD KmdfHelloWorldEvtDeviceAdd;

/*
* DriverEntry is the entry point for all drivers, like Main() is for many user mode applications. 
* The job of DriverEntry is to initialize driver-wide structures and resources. In this example, 
* you printed "Hello World" for DriverEntry, configured the driver object to register your 
* EvtDeviceAdd callback's entry point, then created the driver object and returned.
* 
* The driver object acts as the parent object for all other framework objects you might create in 
* your driver, which include device objects, I/O queues, timers, spinlocks, and more. For more 
* information about framework objects, see Introduction to Framework Objects.
* 
* For DriverEntry, we strongly recommend keeping the name as "DriverEntry" to help with code 
* analysis and debugging.
*/
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT     DriverObject,
    _In_ PUNICODE_STRING    RegistryPath
)
{
    // NTSTATUS variable to record success or failure
    NTSTATUS status = STATUS_SUCCESS;

    // Allocate the driver configuration object
    WDF_DRIVER_CONFIG config;

    // Print "Hello World" for DriverEntry
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfHelloWorld: DriverEntry\n"));

    // Initialize the driver configuration object to register the
    // entry point for the EvtDeviceAdd callback, KmdfHelloWorldEvtDeviceAdd
    WDF_DRIVER_CONFIG_INIT(
        &config,
        KmdfHelloWorldEvtDeviceAdd
    );

    // Finally, create the driver object
    status = WdfDriverCreate(
        DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES,
        &config,
        WDF_NO_HANDLE
    );

    if (NT_SUCCESS(status) == false)
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "KmdfHelloWorld::DriverEntry(): WdfDriverCreate() create\n"));

    return status;
}

/*
* EvtDeviceAdd is invoked by the system when it detects that your device has arrived. Its job 
* is to initialize structuresand resources for that device.In this example, you simply printed 
* out a "Hello World" message for EvtDeviceAdd, created the device object, and returned. In 
* other drivers you write, you might create I / O queues for your hardware, set up a device 
* context storage space for device - specific information, or perform other tasks needed to 
* prepare your device.
* 
* For the device add callback, notice how you named it with your driver's name as a prefix 
* (KmdfHelloWorldEvtDeviceAdd). Generally, we recommend naming your driver's functions in this 
* way to differentiate them from other drivers' functions. DriverEntry is the only one you 
* should name exactly that.
*/
NTSTATUS
KmdfHelloWorldEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    // We're not using the driver object,
    // so we need to mark it as unreferenced
    UNREFERENCED_PARAMETER(Driver);

    NTSTATUS status;

    // Allocate the device object
    WDFDEVICE hDevice;

    // Print "Hello World"
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfHelloWorld: KmdfHelloWorldEvtDeviceAdd\n"));

    // Create the device object
    status = WdfDeviceCreate(&DeviceInit,
        WDF_NO_OBJECT_ATTRIBUTES,
        &hDevice
    );
    return status;
}

/*
* This example illustrates a fundamental concept of drivers: they are a "collection of callbacks" that, 
* once initialized, sit and wait for the system to call them when it needs something. This could be a 
* new device arrival event, an I/O request from a user mode application, a system power shutdown event, 
* a request from another driver, or a surprise removal event when a user unplugs the device unexpectedly. 
* Fortunately, to say "Hello World," you only needed to worry about driver and device creation.
*/