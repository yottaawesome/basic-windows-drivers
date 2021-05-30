#include "NfdCore.hpp"

NDIS_HANDLE         FilterDriverHandle; // NDIS handle for filter driver
NDIS_HANDLE         FilterDriverObject;
NDIS_SPIN_LOCK         FilterListLock;
LIST_ENTRY          FilterModuleList;
NDIS_HANDLE         NdisFilterDeviceHandle = NULL;
PDEVICE_OBJECT      NdisDeviceObject = NULL;

extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT driverObject,
	_In_ PUNICODE_STRING registryPath
);

void DriverUnload(_In_ PDRIVER_OBJECT driverObject)
{
	UNREFERENCED_PARAMETER(driverObject);
}

NTSTATUS
FilterDispatch(
	PDEVICE_OBJECT       DeviceObject,
	PIRP                 Irp
)
{
	PIO_STACK_LOCATION       IrpStack;
	NTSTATUS                 Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(DeviceObject);

	IrpStack = IoGetCurrentIrpStackLocation(Irp);

	switch (IrpStack->MajorFunction)
	{
	case IRP_MJ_CREATE:
		break;

	case IRP_MJ_CLEANUP:
		break;

	case IRP_MJ_CLOSE:
		break;

	default:
		break;
	}

	Irp->IoStatus.Status = Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;
}

_Use_decl_annotations_
NTSTATUS
FilterDeviceIoControl(
	PDEVICE_OBJECT        DeviceObject,
	PIRP                  Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	return STATUS_SUCCESS;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NDIS_STATUS
FilterRegisterDevice(
	VOID
)
{
	NDIS_STATUS            Status = NDIS_STATUS_SUCCESS;
	UNICODE_STRING         DeviceName;
	UNICODE_STRING         DeviceLinkUnicodeString;
	PDRIVER_DISPATCH       DispatchTable[IRP_MJ_MAXIMUM_FUNCTION + 1];
	NDIS_DEVICE_OBJECT_ATTRIBUTES   DeviceAttribute;
	PFILTER_DEVICE_EXTENSION        FilterDeviceExtension;

	//DEBUGP(DL_TRACE, "==>FilterRegisterDevice\n");

	NdisZeroMemory(DispatchTable, (IRP_MJ_MAXIMUM_FUNCTION + 1) * sizeof(PDRIVER_DISPATCH));

	DispatchTable[IRP_MJ_CREATE] = FilterDispatch;
	DispatchTable[IRP_MJ_CLEANUP] = FilterDispatch;
	DispatchTable[IRP_MJ_CLOSE] = FilterDispatch;
	DispatchTable[IRP_MJ_DEVICE_CONTROL] = FilterDeviceIoControl;


	NdisInitUnicodeString(&DeviceName, NTDEVICE_STRING);
	NdisInitUnicodeString(&DeviceLinkUnicodeString, LINKNAME_STRING);

	//
	// Create a device object and register our dispatch handlers
	//
	NdisZeroMemory(&DeviceAttribute, sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES));

	DeviceAttribute.Header.Type = NDIS_OBJECT_TYPE_DEVICE_OBJECT_ATTRIBUTES;
	DeviceAttribute.Header.Revision = NDIS_DEVICE_OBJECT_ATTRIBUTES_REVISION_1;
	DeviceAttribute.Header.Size = sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES);

	DeviceAttribute.DeviceName = &DeviceName;
	DeviceAttribute.SymbolicName = &DeviceLinkUnicodeString;
	DeviceAttribute.MajorFunctions = &DispatchTable[0];
	DeviceAttribute.ExtensionSize = sizeof(FILTER_DEVICE_EXTENSION);

	Status = NdisRegisterDeviceEx(
		FilterDriverHandle,
		&DeviceAttribute,
		&NdisDeviceObject,
		&NdisFilterDeviceHandle
	);


	if (Status == NDIS_STATUS_SUCCESS)
	{
		FilterDeviceExtension = (PFILTER_DEVICE_EXTENSION) NdisGetDeviceReservedExtension(NdisDeviceObject);

		FilterDeviceExtension->Signature = 'FTDR';
		FilterDeviceExtension->Handle = FilterDriverHandle;
	}


	//DEBUGP(DL_TRACE, "<==FilterRegisterDevice: %x\n", Status);

	return (Status);

}

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT driverObject,
	_In_ PUNICODE_STRING registryPath
)
{
	UNREFERENCED_PARAMETER(registryPath);

	FilterDriverObject = driverObject;

	NDIS_STATUS Status = STATUS_SUCCESS;
	NDIS_FILTER_DRIVER_CHARACTERISTICS FChars{};
	NDIS_STRING ServiceName = RTL_CONSTANT_STRING(FILTER_SERVICE_NAME);
	NDIS_STRING UniqueName = RTL_CONSTANT_STRING(FILTER_UNIQUE_NAME);
	NDIS_STRING FriendlyName = RTL_CONSTANT_STRING(FILTER_FRIENDLY_NAME);

	FChars.Header.Type = NDIS_OBJECT_TYPE_FILTER_DRIVER_CHARACTERISTICS;
	FChars.Header.Size = sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS);
	FChars.Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_2;
	FChars.MajorNdisVersion = FILTER_MAJOR_NDIS_VERSION;
	FChars.MinorNdisVersion = FILTER_MINOR_NDIS_VERSION;
	FChars.MajorDriverVersion = 1;
	FChars.MinorDriverVersion = 0;
	FChars.Flags = 0;

	FChars.FriendlyName = FriendlyName;
	FChars.UniqueName = UniqueName;
	FChars.ServiceName = ServiceName;

	//
	// TODO: Most handlers are optional, however, this sample includes them
	// all for illustrative purposes.  If you do not need a particular 
	// handler, set it to NULL and NDIS will more efficiently pass the
	// operation through on your behalf.
	//
	/*FChars.SetOptionsHandler = FilterRegisterOptions;
	FChars.AttachHandler = FilterAttach;
	FChars.DetachHandler = FilterDetach;
	FChars.RestartHandler = FilterRestart;
	FChars.PauseHandler = FilterPause;
	FChars.SetFilterModuleOptionsHandler = FilterSetModuleOptions;
	FChars.OidRequestHandler = FilterOidRequest;
	FChars.OidRequestCompleteHandler = FilterOidRequestComplete;
	FChars.CancelOidRequestHandler = FilterCancelOidRequest;

	FChars.SendNetBufferListsHandler = FilterSendNetBufferLists;
	FChars.ReturnNetBufferListsHandler = FilterReturnNetBufferLists;
	FChars.SendNetBufferListsCompleteHandler = FilterSendNetBufferListsComplete;
	FChars.ReceiveNetBufferListsHandler = FilterReceiveNetBufferLists;
	FChars.DevicePnPEventNotifyHandler = FilterDevicePnPEventNotify;
	FChars.NetPnPEventHandler = FilterNetPnPEvent;
	FChars.StatusHandler = FilterStatus;
	FChars.CancelSendNetBufferListsHandler = FilterCancelSendNetBufferLists;*/

	driverObject->DriverUnload = DriverUnload;

	FilterDriverHandle = NULL;

	//
	// Initialize spin locks
	//
	NdisAllocateSpinLock(&FilterListLock);

	InitializeListHead(&FilterModuleList);

	Status = NdisFRegisterFilterDriver(
		driverObject,
		(NDIS_HANDLE)FilterDriverObject,
		&FChars,
		&FilterDriverHandle);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		//DEBUGP(DL_WARN, "Register filter driver failed.\n");
	}

	Status = FilterRegisterDevice();

	if (Status != NDIS_STATUS_SUCCESS)
	{
		NdisFDeregisterFilterDriver(FilterDriverHandle);
		NdisFreeSpinLock(&FilterListLock);
		//DEBUGP(DL_WARN, "Register device for the filter driver failed.\n");
	}

	return Status;
}
