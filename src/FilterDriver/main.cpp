#include "NfdCore.hpp"

NDIS_HANDLE         FilterDriverHandle; // NDIS handle for filter driver
NDIS_HANDLE         FilterDriverObject;
FILTER_LOCK         FilterListLock;
LIST_ENTRY          FilterModuleList;
NDIS_HANDLE         NdisFilterDeviceHandle = NULL;
PDEVICE_OBJECT      NdisDeviceObject = NULL;

_Use_decl_annotations_
VOID FilterSendNetBufferListsComplete(
	NDIS_HANDLE         FilterModuleContext,
	PNET_BUFFER_LIST    NetBufferLists,
	ULONG               SendCompleteFlags
)
{
	PMS_FILTER         pFilter = (PMS_FILTER)FilterModuleContext;
	ULONG              NumOfSendCompletes = 0;
	BOOLEAN            DispatchLevel;
	PNET_BUFFER_LIST   CurrNbl;

	//
	// If your filter injected any send packets into the datapath to be sent,
	// you must identify their NBLs here and remove them from the chain.  Do not
	// attempt to send-complete your NBLs up to the higher layer.
	//

	//
	// If your filter has modified any NBLs (or NBs, MDLs, etc) in your
	// FilterSendNetBufferLists handler, you must undo the modifications here.
	// In general, NBLs must be returned in the same condition in which you had
	// you received them.  (Exceptions: the NBLs can be re-ordered on the linked
	// list, and the scratch fields are don't-care).
	//

	if (pFilter->TrackSends)
	{
		CurrNbl = NetBufferLists;
		while (CurrNbl)
		{
			NumOfSendCompletes++;
			CurrNbl = NET_BUFFER_LIST_NEXT_NBL(CurrNbl);

		}
		DispatchLevel = NDIS_TEST_SEND_AT_DISPATCH_LEVEL(SendCompleteFlags);
		FILTER_ACQUIRE_LOCK(&pFilter->Lock, DispatchLevel);
		pFilter->OutstandingSends -= NumOfSendCompletes;
		FILTER_LOG_SEND_REF(2, pFilter, PrevNbl, pFilter->OutstandingSends);
		FILTER_RELEASE_LOCK(&pFilter->Lock, DispatchLevel);
	}

	// Send complete the NBLs.  If you removed any NBLs from the chain, make
	// sure the chain isn't empty (i.e., NetBufferLists!=NULL).

	NdisFSendNetBufferListsComplete(pFilter->FilterHandle, NetBufferLists, SendCompleteFlags);
}

_Use_decl_annotations_
VOID
FilterSendNetBufferLists(
	NDIS_HANDLE         FilterModuleContext,
	PNET_BUFFER_LIST    NetBufferLists,
	NDIS_PORT_NUMBER    PortNumber,
	ULONG               SendFlags
)
{
	PMS_FILTER          pFilter = (PMS_FILTER)FilterModuleContext;
	PNET_BUFFER_LIST    CurrNbl;
	BOOLEAN             DispatchLevel;
	BOOLEAN             bFalse = FALSE;

	do
	{

		DispatchLevel = NDIS_TEST_SEND_AT_DISPATCH_LEVEL(SendFlags);
#if DBG
		//
		// we should never get packets to send if we are not in running state
		//

		FILTER_ACQUIRE_LOCK(&pFilter->Lock, DispatchLevel);
		//
		// If the filter is not in running state, fail the send
		//
		if (pFilter->State != FilterRunning)
		{
			FILTER_RELEASE_LOCK(&pFilter->Lock, DispatchLevel);

			CurrNbl = NetBufferLists;
			while (CurrNbl)
			{
				NET_BUFFER_LIST_STATUS(CurrNbl) = NDIS_STATUS_PAUSED;
				CurrNbl = NET_BUFFER_LIST_NEXT_NBL(CurrNbl);
			}
			NdisFSendNetBufferListsComplete(pFilter->FilterHandle,
				NetBufferLists,
				DispatchLevel ? NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL : 0);
			break;

		}
		FILTER_RELEASE_LOCK(&pFilter->Lock, DispatchLevel);
#endif
		if (pFilter->TrackSends)
		{
			FILTER_ACQUIRE_LOCK(&pFilter->Lock, DispatchLevel);
			CurrNbl = NetBufferLists;
			while (CurrNbl)
			{
				pFilter->OutstandingSends++;
				FILTER_LOG_SEND_REF(1, pFilter, CurrNbl, pFilter->OutstandingSends);

				CurrNbl = NET_BUFFER_LIST_NEXT_NBL(CurrNbl);
			}
			FILTER_RELEASE_LOCK(&pFilter->Lock, DispatchLevel);
		}

		//
		// If necessary, queue the NetBufferLists in a local structure for later
		// processing.  However, do not queue them for "too long", or else the
		// system's performance may be degraded.  If you need to hold onto an
		// NBL for an unbounded amount of time, then allocate memory, perform a
		// deep copy, and complete the original NBL.
		//

		NdisFSendNetBufferLists(pFilter->FilterHandle, NetBufferLists, PortNumber, SendFlags);


	} while (bFalse);
}

_Use_decl_annotations_
VOID
FilterReceiveNetBufferLists(
	NDIS_HANDLE         FilterModuleContext,
	PNET_BUFFER_LIST    NetBufferLists,
	NDIS_PORT_NUMBER    PortNumber,
	ULONG               NumberOfNetBufferLists,
	ULONG               ReceiveFlags
)
{

	PMS_FILTER          pFilter = (PMS_FILTER)FilterModuleContext;
	BOOLEAN             DispatchLevel;
	ULONG               Ref;
	BOOLEAN             bFalse = FALSE;
#if DBG
	ULONG               ReturnFlags;
#endif
	do
	{

		DispatchLevel = NDIS_TEST_RECEIVE_AT_DISPATCH_LEVEL(ReceiveFlags);
#if DBG
		FILTER_ACQUIRE_LOCK(&pFilter->Lock, DispatchLevel);

		if (pFilter->State != FilterRunning)
		{
			FILTER_RELEASE_LOCK(&pFilter->Lock, DispatchLevel);

			if (NDIS_TEST_RECEIVE_CAN_PEND(ReceiveFlags))
			{
				ReturnFlags = 0;
				if (NDIS_TEST_RECEIVE_AT_DISPATCH_LEVEL(ReceiveFlags))
				{
					NDIS_SET_RETURN_FLAG(ReturnFlags, NDIS_RETURN_FLAGS_DISPATCH_LEVEL);
				}

				NdisFReturnNetBufferLists(pFilter->FilterHandle, NetBufferLists, ReturnFlags);
			}
			break;
		}
		FILTER_RELEASE_LOCK(&pFilter->Lock, DispatchLevel);
#endif

		ASSERT(NumberOfNetBufferLists >= 1);

		//
		// If you would like to drop a received packet, then you must carefully
		// modify the NBL chain as follows:
		//
		//     if NDIS_TEST_RECEIVE_CANNOT_PEND(ReceiveFlags):
		//         For each NBL that is NOT dropped, temporarily unlink it from
		//         the linked list, and indicate it up alone with 
		//         NdisFIndicateReceiveNetBufferLists and the
		//         NDIS_RECEIVE_FLAGS_RESOURCES flag set.  Then immediately
		//         relink the NBL back into the chain.  When all NBLs have been
		//         indicated up, you may return from this function.
		//     otherwise (NDIS_TEST_RECEIVE_CANNOT_PEND is FALSE):
		//         Divide the linked list of NBLs into two chains: one chain
		//         of packets to drop, and everything else in another chain.
		//         Return the first chain with NdisFReturnNetBufferLists, and
		//         indicate up the rest with NdisFIndicateReceiveNetBufferLists.
		//
		// Note: on the receive path for Ethernet packets, one NBL will have 
		// exactly one NB.  So (assuming you are receiving on Ethernet, or are 
		// attached above Native WiFi) you do not need to worry about dropping
		// one NB, but trying to indicate up the remaining NBs on the same NBL.
		// In other words, if the first NB should be dropped, drop the whole NBL.
		//

		//
		// If you would like to modify a packet, and can do so quickly, you may
		// do it here.  However, make sure you save enough information to undo
		// your modification in the FilterReturnNetBufferLists handler.
		//

		//
		// If necessary, queue the NetBufferLists in a local structure for later
		// processing.  However, do not queue them for "too long", or else the
		// system's performance may be degraded.  If you need to hold onto an
		// NBL for an unbounded amount of time, then allocate memory, perform a
		// deep copy, and return the original NBL.
		//

		if (pFilter->TrackReceives)
		{
			FILTER_ACQUIRE_LOCK(&pFilter->Lock, DispatchLevel);
			pFilter->OutstandingRcvs += NumberOfNetBufferLists;
			Ref = pFilter->OutstandingRcvs;

			FILTER_LOG_RCV_REF(1, pFilter, NetBufferLists, Ref);
			FILTER_RELEASE_LOCK(&pFilter->Lock, DispatchLevel);
		}

		NdisFIndicateReceiveNetBufferLists(
			pFilter->FilterHandle,
			NetBufferLists,
			PortNumber,
			NumberOfNetBufferLists,
			ReceiveFlags);


		if (NDIS_TEST_RECEIVE_CANNOT_PEND(ReceiveFlags) &&
			pFilter->TrackReceives)
		{
			FILTER_ACQUIRE_LOCK(&pFilter->Lock, DispatchLevel);
			pFilter->OutstandingRcvs -= NumberOfNetBufferLists;
			Ref = pFilter->OutstandingRcvs;
			FILTER_LOG_RCV_REF(2, pFilter, NetBufferLists, Ref);
			FILTER_RELEASE_LOCK(&pFilter->Lock, DispatchLevel);
		}

	} while (bFalse);
}

_Use_decl_annotations_
VOID
FilterReturnNetBufferLists(
	NDIS_HANDLE         FilterModuleContext,
	PNET_BUFFER_LIST    NetBufferLists,
	ULONG               ReturnFlags
)
{
	PMS_FILTER          pFilter = (PMS_FILTER)FilterModuleContext;
	PNET_BUFFER_LIST    CurrNbl = NetBufferLists;
	UINT                NumOfNetBufferLists = 0;
	BOOLEAN             DispatchLevel;
	ULONG               Ref;

	//
	// If your filter injected any receive packets into the datapath to be
	// received, you must identify their NBLs here and remove them from the 
	// chain.  Do not attempt to receive-return your NBLs down to the lower
	// layer.
	//

	//
	// If your filter has modified any NBLs (or NBs, MDLs, etc) in your
	// FilterReceiveNetBufferLists handler, you must undo the modifications here.
	// In general, NBLs must be returned in the same condition in which you had
	// you received them.  (Exceptions: the NBLs can be re-ordered on the linked
	// list, and the scratch fields are don't-care).
	//

	if (pFilter->TrackReceives)
	{
		while (CurrNbl)
		{
			NumOfNetBufferLists++;
			CurrNbl = NET_BUFFER_LIST_NEXT_NBL(CurrNbl);
		}
	}


	// Return the received NBLs.  If you removed any NBLs from the chain, make
	// sure the chain isn't empty (i.e., NetBufferLists!=NULL).

	NdisFReturnNetBufferLists(pFilter->FilterHandle, NetBufferLists, ReturnFlags);

	if (pFilter->TrackReceives)
	{
		DispatchLevel = NDIS_TEST_RETURN_AT_DISPATCH_LEVEL(ReturnFlags);
		FILTER_ACQUIRE_LOCK(&pFilter->Lock, DispatchLevel);

		pFilter->OutstandingRcvs -= NumOfNetBufferLists;
		Ref = pFilter->OutstandingRcvs;
		FILTER_LOG_RCV_REF(3, pFilter, NetBufferLists, Ref);
		FILTER_RELEASE_LOCK(&pFilter->Lock, DispatchLevel);
	}
}

NDIS_FILTER_PARTIAL_CHARACTERISTICS DefaultChars = {
{ 0, 0, 0},
	  0,
	  FilterSendNetBufferLists,
	  FilterSendNetBufferListsComplete,
	  NULL,
	  FilterReceiveNetBufferLists,
	  FilterReturnNetBufferLists
};

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
NDIS_STATUS
FilterRegisterOptions(
	NDIS_HANDLE  NdisFilterDriverHandle,
	NDIS_HANDLE  FilterDriverContext
)
{

	ASSERT(NdisFilterDriverHandle == FilterDriverHandle);
	ASSERT(FilterDriverContext == (NDIS_HANDLE)FilterDriverObject);

	if ((NdisFilterDriverHandle != (NDIS_HANDLE)FilterDriverHandle) ||
		(FilterDriverContext != (NDIS_HANDLE)FilterDriverObject))
	{
		return NDIS_STATUS_INVALID_PARAMETER;
	}

	return NDIS_STATUS_SUCCESS;
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

_Use_decl_annotations_
NDIS_STATUS
FilterSetModuleOptions(
	NDIS_HANDLE             FilterModuleContext
)
{
	PMS_FILTER                               pFilter = (PMS_FILTER)FilterModuleContext;
	NDIS_FILTER_PARTIAL_CHARACTERISTICS      OptionalHandlers;
	NDIS_STATUS                              Status = NDIS_STATUS_SUCCESS;
	BOOLEAN                                  bFalse = FALSE;

	//
	// Demonstrate how to change send/receive handlers at runtime.
	//
	if (bFalse)
	{
		UINT      i;


		pFilter->CallsRestart++;

		i = pFilter->CallsRestart % 8;

		pFilter->TrackReceives = TRUE;
		pFilter->TrackSends = TRUE;

		NdisMoveMemory(&OptionalHandlers, &DefaultChars, sizeof(OptionalHandlers));
		OptionalHandlers.Header.Type = NDIS_OBJECT_TYPE_FILTER_PARTIAL_CHARACTERISTICS;
		OptionalHandlers.Header.Size = sizeof(OptionalHandlers);
		switch (i)
		{

		case 0:
			OptionalHandlers.ReceiveNetBufferListsHandler = NULL;
			pFilter->TrackReceives = FALSE;
			break;

		case 1:

			OptionalHandlers.ReturnNetBufferListsHandler = NULL;
			pFilter->TrackReceives = FALSE;
			break;

		case 2:
			OptionalHandlers.SendNetBufferListsHandler = NULL;
			pFilter->TrackSends = FALSE;
			break;

		case 3:
			OptionalHandlers.SendNetBufferListsCompleteHandler = NULL;
			pFilter->TrackSends = FALSE;
			break;

		case 4:
			OptionalHandlers.ReceiveNetBufferListsHandler = NULL;
			OptionalHandlers.ReturnNetBufferListsHandler = NULL;
			break;

		case 5:
			OptionalHandlers.SendNetBufferListsHandler = NULL;
			OptionalHandlers.SendNetBufferListsCompleteHandler = NULL;
			break;

		case 6:

			OptionalHandlers.ReceiveNetBufferListsHandler = NULL;
			OptionalHandlers.ReturnNetBufferListsHandler = NULL;
			OptionalHandlers.SendNetBufferListsHandler = NULL;
			OptionalHandlers.SendNetBufferListsCompleteHandler = NULL;
			break;

		case 7:
			break;
		}
		Status = NdisSetOptionalHandlers(pFilter->FilterHandle, (PNDIS_DRIVER_OPTIONAL_HANDLERS)&OptionalHandlers);
	}
	return Status;
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
	//FChars.SetOptionsHandler = FilterRegisterOptions;
	//FChars.SetFilterModuleOptionsHandler = FilterSetModuleOptions;
	//FChars.AttachHandler = FilterAttach;
	//FChars.DetachHandler = FilterDetach;
	//FChars.RestartHandler = FilterRestart;
	//FChars.PauseHandler = FilterPause;

	/*
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
	FILTER_INIT_LOCK(&FilterListLock);

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
		FILTER_FREE_LOCK(&FilterListLock);
		//DEBUGP(DL_WARN, "Register device for the filter driver failed.\n");
	}

	return Status;
}
