module;

#include <ntifs.h>
#include <ntddk.h>
#include "PriorityBoosterCommon.hpp"

module priority_booster;

namespace PriorityBooster
{
	void DriverUnload(_In_ PDRIVER_OBJECT driverObject)
	{
		UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\PriorityBooster");
		// delete symbolic link
		IoDeleteSymbolicLink(&symLink);

		// delete device object
		IoDeleteDevice(driverObject->DeviceObject);
	}

	// The function must return NTSTATUS and accepts a pointer to a device object and a pointer to an I/O 
	// Request Packet(IRP). An IRP is the primary object where the request information is stored, for all
	// types of requests.
	NTSTATUS PriorityBoosterCreateClose(_In_ PDEVICE_OBJECT deviceObject, _In_ PIRP irp)
	{
		UNREFERENCED_PARAMETER(deviceObject);

		irp->IoStatus.Status = STATUS_SUCCESS;
		irp->IoStatus.Information = 0;
		// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iocompleterequest
		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return STATUS_SUCCESS;
	}

	NTSTATUS PriorityBoosterDeviceControl(_In_ PDEVICE_OBJECT deviceObject, _In_ PIRP irp)
	{
		UNREFERENCED_PARAMETER(deviceObject);

		// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-iogetcurrentirpstacklocation
		PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
		NTSTATUS status = STATUS_SUCCESS;

		// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_io_stack_location
		switch (stack->Parameters.DeviceIoControl.IoControlCode)
		{
			case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY:
			{
				if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadData))
				{
					status = STATUS_BUFFER_TOO_SMALL;
					break;
				}

				ThreadData* data = (ThreadData*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
				if (data == nullptr)
				{
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				if (data->Priority < 1 || data->Priority>31)
				{
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				PETHREAD thread = nullptr;
				// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-pslookupthreadbythreadid
				status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &thread);
				if (NT_SUCCESS(status) == false)
					break;

				// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-kesetprioritythread
				KeSetPriorityThread((PKTHREAD)thread, data->Priority);
				// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-obdereferenceobject
				ObDereferenceObject(thread);

				break;
			}

			default:
				status = STATUS_INVALID_DEVICE_REQUEST;
		}

		irp->IoStatus.Status = status;
		irp->IoStatus.Information = 0;
		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return status;
	}
}
