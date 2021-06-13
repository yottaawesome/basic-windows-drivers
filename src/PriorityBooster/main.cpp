#include <ntifs.h>
#include <ntddk.h>
#include "PriorityBoosterCommon.hpp"

import priority_booster;


extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING registryPath);

/*
* Most software drivers need to do the following in DriverEntry():
* •Set an Unload routine.
* •Set dispatch routines the driver supports.
* •Create a device object.
* •Create a symbolic link to the device object.
*/
extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT driverObject, 
	_In_ PUNICODE_STRING registryPath
)
{
	UNREFERENCED_PARAMETER(registryPath);

	driverObject->DriverUnload = (PDRIVER_UNLOAD)PriorityBooster::DriverUnload;

	// Next, we need to set up the dispatch routines that we want to support. Practically all drivers 
	// mustsupport IRP_MJ_CREATE and IRP_MJ_CLOSE, otherwise there would be no way to open a handle to
	// any device for this driver
	driverObject->MajorFunction[IRP_MJ_CREATE] = (PDRIVER_DISPATCH)PriorityBooster::PriorityBoosterCreateClose;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = (PDRIVER_DISPATCH)PriorityBooster::PriorityBoosterCreateClose;
	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = (PDRIVER_DISPATCH)PriorityBooster::PriorityBoosterDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\PriorityBooster");
	PDEVICE_OBJECT deviceObject = nullptr;
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatedevice
	NTSTATUS status = IoCreateDevice(
		driverObject,
		0,
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		false,
		&deviceObject
	);
	if (NT_SUCCESS(status) == false)
	{
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PriorityBooster::DriverEntry(): IoCreateDevice() failed\n"));
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\PriorityBooster");
	// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocreatesymboliclink
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (NT_SUCCESS(status) == false)
	{
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PriorityBooster::DriverEntry(): IoCreateSymbolicLink() failed\n"));
		return status;
	}

	return STATUS_SUCCESS;
}
