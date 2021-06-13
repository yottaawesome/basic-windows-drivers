module;

#include <ntifs.h>
#include <ntddk.h>

export module priority_booster;

export namespace PriorityBooster
{
	void DriverUnload(_In_ PDRIVER_OBJECT driverObject);
	NTSTATUS PriorityBoosterCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
	NTSTATUS PriorityBoosterDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
}