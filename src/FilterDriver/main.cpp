#include "NfdCore.hpp"

NDIS_HANDLE         FilterDriverHandle; // NDIS handle for filter driver
NDIS_HANDLE         FilterDriverObject;
FILTER_LOCK         FilterListLock;
LIST_ENTRY          FilterModuleList;
NDIS_HANDLE         NdisFilterDeviceHandle = NULL;
PDEVICE_OBJECT      NdisDeviceObject = NULL;

_Use_decl_annotations_
NDIS_STATUS
FilterPause(
	NDIS_HANDLE                     FilterModuleContext,
	PNDIS_FILTER_PAUSE_PARAMETERS   PauseParameters
)
/*++

Routine Description:

	Filter pause routine.
	Complete all the outstanding sends and queued sends,
	wait for all the outstanding recvs to be returned
	and return all the queued receives.

Arguments:

	FilterModuleContext - pointer to the filter context stucture
	PauseParameters     - additional information about the pause

Return Value:

	NDIS_STATUS_SUCCESS if filter pauses successfully, NDIS_STATUS_PENDING
	if not.  No other return value is allowed (pause must succeed, eventually).

N.B.: When the filter is in Pausing state, it can still process OID requests,
	complete sending, and returning packets to NDIS, and also indicate status.
	After this function completes, the filter must not attempt to send or
	receive packets, but it may still process OID requests and status
	indications.

--*/
{
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FilterPause()\n"));
	PMS_FILTER          pFilter = (PMS_FILTER)(FilterModuleContext);
	NDIS_STATUS         Status;
	BOOLEAN               bFalse = FALSE;

	UNREFERENCED_PARAMETER(PauseParameters);

	//DEBUGP(DL_TRACE, "===>NDISLWF FilterPause: FilterInstance %p\n", FilterModuleContext);

	//
	// Set the flag that the filter is going to pause
	//
	FILTER_ASSERT(pFilter->State == FilterRunning);

	FILTER_ACQUIRE_LOCK(&pFilter->Lock, bFalse);
	pFilter->State = FilterPausing;
	FILTER_RELEASE_LOCK(&pFilter->Lock, bFalse);

	//
	// Do whatever work is required to bring the filter into the Paused state.
	//
	// If you have diverted and queued any send or receive NBLs, return them 
	// now.
	//
	// If you send or receive original NBLs, stop doing that and wait for your
	// NBLs to return to you now.
	//


	Status = NDIS_STATUS_SUCCESS;

	pFilter->State = FilterPaused;

	//DEBUGP(DL_TRACE, "<===FilterPause:  Status %x\n", Status);
	return Status;
}

_Use_decl_annotations_
NDIS_STATUS
FilterAttach(
	NDIS_HANDLE                     NdisFilterHandle,
	NDIS_HANDLE                     FilterDriverContext,
	PNDIS_FILTER_ATTACH_PARAMETERS  AttachParameters
)
/*++

Routine Description:

	Filter attach routine.
	Create filter's context, allocate NetBufferLists and NetBuffer pools and any
	other resources, and read configuration if needed.

Arguments:

	NdisFilterHandle - Specify a handle identifying this instance of the filter. FilterAttach
					   should save this handle. It is a required  parameter in subsequent calls
					   to NdisFxxx functions.
	FilterDriverContext - Filter driver context passed to NdisFRegisterFilterDriver.

	AttachParameters - attach parameters

Return Value:

	NDIS_STATUS_SUCCESS: FilterAttach successfully allocated and initialize data structures
						 for this filter instance.
	NDIS_STATUS_RESOURCES: FilterAttach failed due to insufficient resources.
	NDIS_STATUS_FAILURE: FilterAttach could not set up this instance of this filter and it has called
						 NdisWriteErrorLogEntry with parameters specifying the reason for failure.

N.B.:  FILTER can use NdisRegisterDeviceEx to create a device, so the upper
	layer can send Irps to the filter.

--*/
{
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FilterAttach()\n"));

	PMS_FILTER              pFilter = NULL;
	NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
	NDIS_FILTER_ATTRIBUTES  FilterAttributes;
	ULONG                   Size;
	BOOLEAN               bFalse = FALSE;

	//DEBUGP(DL_TRACE, "===>FilterAttach: NdisFilterHandle %p\n", NdisFilterHandle);

	do
	{
		ASSERT(FilterDriverContext == (NDIS_HANDLE)FilterDriverObject);
		if (FilterDriverContext != (NDIS_HANDLE)FilterDriverObject)
		{
			Status = NDIS_STATUS_INVALID_PARAMETER;
			break;
		}

		// Verify the media type is supported.  This is a last resort; the
		// the filter should never have been bound to an unsupported miniport
		// to begin with.  If this driver is marked as a Mandatory filter (which
		// is the default for this sample; see the INF file), failing to attach 
		// here will leave the network adapter in an unusable state.
		//
		// Your setup/install code should not bind the filter to unsupported
		// media types.
		if ((AttachParameters->MiniportMediaType != NdisMedium802_3)
			&& (AttachParameters->MiniportMediaType != NdisMediumWan)
			&& (AttachParameters->MiniportMediaType != NdisMediumWirelessWan))
		{
			//DEBUGP(DL_ERROR, "Unsupported media type.\n");

			Status = NDIS_STATUS_INVALID_PARAMETER;
			break;
		}

		Size = sizeof(MS_FILTER) +
			AttachParameters->FilterModuleGuidName->Length +
			AttachParameters->BaseMiniportInstanceName->Length +
			AttachParameters->BaseMiniportName->Length;

		pFilter = (PMS_FILTER)FILTER_ALLOC_MEM(NdisFilterHandle, Size);
		if (pFilter == NULL)
		{
			//DEBUGP(DL_WARN, "Failed to allocate context structure.\n");
			Status = NDIS_STATUS_RESOURCES;
			break;
		}

		NdisZeroMemory(pFilter, sizeof(MS_FILTER));

		pFilter->FilterModuleName.Length = pFilter->FilterModuleName.MaximumLength = AttachParameters->FilterModuleGuidName->Length;
		pFilter->FilterModuleName.Buffer = (PWSTR)((PUCHAR)pFilter + sizeof(MS_FILTER));
		NdisMoveMemory(pFilter->FilterModuleName.Buffer,
			AttachParameters->FilterModuleGuidName->Buffer,
			pFilter->FilterModuleName.Length);



		pFilter->MiniportFriendlyName.Length = pFilter->MiniportFriendlyName.MaximumLength = AttachParameters->BaseMiniportInstanceName->Length;
		pFilter->MiniportFriendlyName.Buffer = (PWSTR)((PUCHAR)pFilter->FilterModuleName.Buffer + pFilter->FilterModuleName.Length);
		NdisMoveMemory(pFilter->MiniportFriendlyName.Buffer,
			AttachParameters->BaseMiniportInstanceName->Buffer,
			pFilter->MiniportFriendlyName.Length);


		pFilter->MiniportName.Length = pFilter->MiniportName.MaximumLength = AttachParameters->BaseMiniportName->Length;
		pFilter->MiniportName.Buffer = (PWSTR)((PUCHAR)pFilter->MiniportFriendlyName.Buffer +
			pFilter->MiniportFriendlyName.Length);
		NdisMoveMemory(pFilter->MiniportName.Buffer,
			AttachParameters->BaseMiniportName->Buffer,
			pFilter->MiniportName.Length);

		pFilter->MiniportIfIndex = AttachParameters->BaseMiniportIfIndex;
		//
		// The filter should initialize TrackReceives and TrackSends properly. For this
		// driver, since its default characteristic has both a send and a receive handler,
		// these fields are initialized to TRUE.
		//
		pFilter->TrackReceives = TRUE;
		pFilter->TrackSends = TRUE;
		pFilter->FilterHandle = NdisFilterHandle;


		NdisZeroMemory(&FilterAttributes, sizeof(NDIS_FILTER_ATTRIBUTES));
		FilterAttributes.Header.Revision = NDIS_FILTER_ATTRIBUTES_REVISION_1;
		FilterAttributes.Header.Size = sizeof(NDIS_FILTER_ATTRIBUTES);
		FilterAttributes.Header.Type = NDIS_OBJECT_TYPE_FILTER_ATTRIBUTES;
		FilterAttributes.Flags = 0;

		NDIS_DECLARE_FILTER_MODULE_CONTEXT(MS_FILTER);
		Status = NdisFSetAttributes(NdisFilterHandle,
			pFilter,
			&FilterAttributes);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			//DEBUGP(DL_WARN, "Failed to set attributes.\n");
			break;
		}


		pFilter->State = FilterPaused;

		FILTER_ACQUIRE_LOCK(&FilterListLock, bFalse);
		InsertHeadList(&FilterModuleList, &pFilter->FilterModuleLink);
		FILTER_RELEASE_LOCK(&FilterListLock, bFalse);

	} while (bFalse);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pFilter != NULL)
		{
			FILTER_FREE_MEM(pFilter);
		}
	}

	//DEBUGP(DL_TRACE, "<===FilterAttach:    Status %x\n", Status);
	return Status;
}

_Use_decl_annotations_
VOID
FilterDetach(
	NDIS_HANDLE     FilterModuleContext
)
/*++

Routine Description:

	Filter detach routine.
	This is a required function that will deallocate all the resources allocated during
	FilterAttach. NDIS calls FilterAttach to remove a filter instance from a filter stack.

Arguments:

	FilterModuleContext - pointer to the filter context area.

Return Value:
	None.

NOTE: Called at PASSIVE_LEVEL and the filter is in paused state

--*/
{
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FilterDetach()\n"));

	PMS_FILTER                  pFilter = (PMS_FILTER)FilterModuleContext;
	BOOLEAN                      bFalse = FALSE;


	//DEBUGP(DL_TRACE, "===>FilterDetach:    FilterInstance %p\n", FilterModuleContext);


	//
	// Filter must be in paused state
	//
	FILTER_ASSERT(pFilter->State == FilterPaused);


	//
	// Detach must not fail, so do not put any code here that can possibly fail.
	//

	//
	// Free filter instance name if allocated.
	//
	if (pFilter->FilterName.Buffer != NULL)
	{
		FILTER_FREE_MEM(pFilter->FilterName.Buffer);
	}


	FILTER_ACQUIRE_LOCK(&FilterListLock, bFalse);
	RemoveEntryList(&pFilter->FilterModuleLink);
	FILTER_RELEASE_LOCK(&FilterListLock, bFalse);


	//
	// Free the memory allocated
	FILTER_FREE_MEM(pFilter);

	//DEBUGP(DL_TRACE, "<===FilterDetach Successfully\n");
	return;
}

_Use_decl_annotations_
NDIS_STATUS
FilterRestart(
	NDIS_HANDLE                     FilterModuleContext,
	PNDIS_FILTER_RESTART_PARAMETERS RestartParameters
)
/*++

Routine Description:

	Filter restart routine.
	Start the datapath - begin sending and receiving NBLs.

Arguments:

	FilterModuleContext - pointer to the filter context stucture.
	RestartParameters   - additional information about the restart operation.

Return Value:

	NDIS_STATUS_SUCCESS: if filter restarts successfully
	NDIS_STATUS_XXX: Otherwise.

--*/
{
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FilterRestart()\n"));
	NDIS_STATUS     Status;
	PMS_FILTER      pFilter = (PMS_FILTER)FilterModuleContext;
	NDIS_HANDLE     ConfigurationHandle = NULL;


	PNDIS_RESTART_GENERAL_ATTRIBUTES NdisGeneralAttributes;
	PNDIS_RESTART_ATTRIBUTES         NdisRestartAttributes;
	NDIS_CONFIGURATION_OBJECT        ConfigObject;

	//DEBUGP(DL_TRACE, "===>FilterRestart:   FilterModuleContext %p\n", FilterModuleContext);

	FILTER_ASSERT(pFilter->State == FilterPaused);

	ConfigObject.Header.Type = NDIS_OBJECT_TYPE_CONFIGURATION_OBJECT;
	ConfigObject.Header.Revision = NDIS_CONFIGURATION_OBJECT_REVISION_1;
	ConfigObject.Header.Size = sizeof(NDIS_CONFIGURATION_OBJECT);
	ConfigObject.NdisHandle = FilterDriverHandle;
	ConfigObject.Flags = 0;

	Status = NdisOpenConfigurationEx(&ConfigObject, &ConfigurationHandle);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		//
		// Filter driver can choose to fail the restart if it cannot open the configuration
		//

#if 0
		//
		// The code is here just to demonstrate how to call NDIS to write an 
		// event to the eventlog.
		//
		PWCHAR              ErrorString = L"Ndislwf";

		DEBUGP(DL_WARN, "FilterRestart: Cannot open configuration.\n");
		NdisWriteEventLogEntry(FilterDriverObject,
			EVENT_NDIS_DRIVER_FAILURE,
			0,
			1,
			&ErrorString,
			sizeof(Status),
			&Status);
#endif

	}

	//
	// This sample doesn't actually do anything with the configuration handle;
	// it is opened here for illustrative purposes.  If you do not need to
	// read configuration, you may omit the code manipulating the 
	// ConfigurationHandle.
	//

	if (Status == NDIS_STATUS_SUCCESS)
	{
		NdisCloseConfiguration(ConfigurationHandle);
	}

	NdisRestartAttributes = RestartParameters->RestartAttributes;

	//
	// If NdisRestartAttributes is not NULL, then the filter can modify generic 
	// attributes and add new media specific info attributes at the end. 
	// Otherwise, if NdisRestartAttributes is NULL, the filter should not try to 
	// modify/add attributes.
	//
	if (NdisRestartAttributes != NULL)
	{
		PNDIS_RESTART_ATTRIBUTES   NextAttributes;

		ASSERT(NdisRestartAttributes->Oid == OID_GEN_MINIPORT_RESTART_ATTRIBUTES);

		NdisGeneralAttributes = (PNDIS_RESTART_GENERAL_ATTRIBUTES)NdisRestartAttributes->Data;

		//
		// Check to see if we need to change any attributes. For example, the
		// driver can change the current MAC address here. Or the driver can add
		// media specific info attributes.
		//
		NdisGeneralAttributes->LookaheadSize = 128;

		//
		// Check each attribute to see whether the filter needs to modify it.
		//
		NextAttributes = NdisRestartAttributes->Next;

		while (NextAttributes != NULL)
		{
			//
			// If somehow the filter needs to change a attributes which requires more space then
			// the current attributes:
			// 1. Remove the attribute from the Attributes list:
			//    TempAttributes = NextAttributes;
			//    NextAttributes = NextAttributes->Next;
			// 2. Free the memory for the current attributes: NdisFreeMemory(TempAttributes, 0 , 0);
			// 3. Dynamically allocate the memory for the new attributes by calling
			//    NdisAllocateMemoryWithTagPriority:
			//    NewAttributes = NdisAllocateMemoryWithTagPriority(Handle, size, Priority);
			// 4. Fill in the new attribute
			// 5. NewAttributes->Next = NextAttributes;
			// 6. NextAttributes = NewAttributes; // Just to make the next statement work.
			//
			NextAttributes = NextAttributes->Next;
		}

		//
		// Add a new attributes at the end
		// 1. Dynamically allocate the memory for the new attributes by calling
		//    NdisAllocateMemoryWithTagPriority.
		// 2. Fill in the new attribute
		// 3. NextAttributes->Next = NewAttributes;
		// 4. NewAttributes->Next = NULL;



	}

	//
	// If everything is OK, set the filter in running state.
	//
	pFilter->State = FilterRunning; // when successful


	Status = NDIS_STATUS_SUCCESS;


	//
	// Ensure the state is Paused if restart failed.
	//

	if (Status != NDIS_STATUS_SUCCESS)
	{
		pFilter->State = FilterPaused;
	}

	//DEBUGP(DL_TRACE, "<===FilterRestart:  FilterModuleContext %p, Status %x\n", FilterModuleContext, Status);
	return Status;
}

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

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
FilterDeregisterDevice(
	VOID
)
{
	if (NdisFilterDeviceHandle != NULL)
	{
		NdisDeregisterDeviceEx(NdisFilterDeviceHandle);
	}

	NdisFilterDeviceHandle = NULL;
}

void DriverUnload(_In_ PDRIVER_OBJECT driverObject)
{
	UNREFERENCED_PARAMETER(driverObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DriverUnload()\n"));


#if DBG
	BOOLEAN               bFalse = FALSE;
#endif

	//DEBUGP(DL_TRACE, "===>FilterUnload\n");

	//
	// Should free the filter context list
	//
	FilterDeregisterDevice();
	NdisFDeregisterFilterDriver(FilterDriverHandle);

#if DBG
	FILTER_ACQUIRE_LOCK(&FilterListLock, bFalse);
	ASSERT(IsListEmpty(&FilterModuleList));

	FILTER_RELEASE_LOCK(&FilterListLock, bFalse);

#endif

	FILTER_FREE_LOCK(&FilterListLock);

	//DEBUGP(DL_TRACE, "<===FilterUnload\n");

	return;
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
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FilterRegisterOptions()\n"));

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
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FilterSetModuleOptions()\n"));

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

_Use_decl_annotations_
VOID
FilterDevicePnPEventNotify(
	NDIS_HANDLE             FilterModuleContext,
	PNET_DEVICE_PNP_EVENT   NetDevicePnPEvent
)
/*++

Routine Description:

	Device PNP event handler

Arguments:

	FilterModuleContext         - our filter context
	NetDevicePnPEvent           - a Device PnP event

NOTE: called at PASSIVE_LEVEL

--*/
{
	PMS_FILTER             pFilter = (PMS_FILTER)FilterModuleContext;
	NDIS_DEVICE_PNP_EVENT  DevicePnPEvent = NetDevicePnPEvent->DevicePnPEvent;
#if DBG
	BOOLEAN                bFalse = FALSE;
#endif

	//DEBUGP(DL_TRACE, "===>FilterDevicePnPEventNotify: NetPnPEvent = %p.\n", NetDevicePnPEvent);

	//
	// The filter may do processing on the event here, including intercepting
	// and dropping it entirely.  However, the sample does nothing with Device
	// PNP events, except pass them down to the next lower* layer.  It is more
	// efficient to omit the FilterDevicePnPEventNotify handler entirely if it
	// does nothing, but it is included in this sample for illustrative purposes.
	//
	// * Trivia: Device PNP events percolate DOWN the stack, instead of upwards
	// like status indications and Net PNP events.  So the next layer is the
	// LOWER layer.
	//

	switch (DevicePnPEvent)
	{

	case NdisDevicePnPEventQueryRemoved:
	case NdisDevicePnPEventRemoved:
	case NdisDevicePnPEventSurpriseRemoved:
	case NdisDevicePnPEventQueryStopped:
	case NdisDevicePnPEventStopped:
	case NdisDevicePnPEventPowerProfileChanged:
	case NdisDevicePnPEventFilterListChanged:

		break;

	default:
		//DEBUGP(DL_ERROR, "FilterDevicePnPEventNotify: Invalid event.\n");
		FILTER_ASSERT(bFalse);

		break;
	}

	NdisFDevicePnPEventNotify(pFilter->FilterHandle, NetDevicePnPEvent);

	//DEBUGP(DL_TRACE, "<===FilterDevicePnPEventNotify\n");

}

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT driverObject,
	_In_ PUNICODE_STRING registryPath
)
{
	NDIS_STATUS Status;
	NDIS_FILTER_DRIVER_CHARACTERISTICS      FChars;
	NDIS_STRING ServiceName = RTL_CONSTANT_STRING(FILTER_SERVICE_NAME);
	NDIS_STRING UniqueName = RTL_CONSTANT_STRING(FILTER_UNIQUE_NAME);
	NDIS_STRING FriendlyName = RTL_CONSTANT_STRING(FILTER_FRIENDLY_NAME);
	BOOLEAN bFalse = FALSE;

	UNREFERENCED_PARAMETER(registryPath);

	//DEBUGP(DL_TRACE, "===>DriverEntry...\n");

	FilterDriverObject = driverObject;

	do
	{
		NdisZeroMemory(&FChars, sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS));
		FChars.Header.Type = NDIS_OBJECT_TYPE_FILTER_DRIVER_CHARACTERISTICS;
		FChars.Header.Size = sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS);
#if NDIS_SUPPORT_NDIS61
		FChars.Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_2;
#else
		FChars.Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_1;
#endif

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
		FChars.SetOptionsHandler = FilterRegisterOptions;
		FChars.AttachHandler = FilterAttach;
		FChars.DetachHandler = FilterDetach;
		FChars.RestartHandler = FilterRestart;
		FChars.PauseHandler = FilterPause;
		FChars.SetFilterModuleOptionsHandler = FilterSetModuleOptions;
		//FChars.OidRequestHandler = FilterOidRequest;
		//FChars.OidRequestCompleteHandler = FilterOidRequestComplete;
		//FChars.CancelOidRequestHandler = FilterCancelOidRequest;

		FChars.SendNetBufferListsHandler = FilterSendNetBufferLists;
		FChars.ReturnNetBufferListsHandler = FilterReturnNetBufferLists;
		FChars.SendNetBufferListsCompleteHandler = FilterSendNetBufferListsComplete;
		FChars.ReceiveNetBufferListsHandler = FilterReceiveNetBufferLists;
		FChars.DevicePnPEventNotifyHandler = FilterDevicePnPEventNotify;
		//FChars.NetPnPEventHandler = FilterNetPnPEvent;
		//FChars.StatusHandler = FilterStatus;
		//FChars.CancelSendNetBufferListsHandler = FilterCancelSendNetBufferLists;

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
			&FilterDriverHandle
		);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			//DEBUGP(DL_WARN, "Register filter driver failed.\n");
			FILTER_FREE_LOCK(&FilterListLock);
			break;
		}

		Status = FilterRegisterDevice();

		if (Status != NDIS_STATUS_SUCCESS)
		{
			NdisFDeregisterFilterDriver(FilterDriverHandle);
			FILTER_FREE_LOCK(&FilterListLock);
			//DEBUGP(DL_WARN, "Register device for the filter driver failed.\n");
			break;
		}
	} while (bFalse);

	//DEBUGP(DL_TRACE, "<===DriverEntry, Status = %8x\n", Status);
	return Status;
}
