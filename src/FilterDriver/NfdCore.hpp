#pragma once
#pragma warning(disable:4201)  //nonstandard extension used : nameless struct/union
//#include <ntddk.h>
#include <ndis.h>

typedef NDIS_SPIN_LOCK      FILTER_LOCK;
typedef PNDIS_SPIN_LOCK     PFILTER_LOCK;

#define FILTER_ASSERT(exp)                                              \
        {                                                               \
            if (!(exp))                                                 \
            {                                                           \
                DbgPrint("Filter: assert " #exp " failed in"            \
                    " file %s, line %d\n", __FILE__, __LINE__);         \
                DbgBreakPoint();                                        \
            }                                                           \
        }

#define FILTER_ALLOC_TAG           'tliF'

#define FILTER_LOG_RCV_REF(_O, _Instance, _NetBufferList, _Ref)

#define FILTER_INIT_LOCK(_pLock)      NdisAllocateSpinLock(_pLock)

#define FILTER_FREE_LOCK(_pLock)      NdisFreeSpinLock(_pLock)

#define FILTER_LOG_SEND_REF(_O, _Instance, _NetBufferList, _Ref)

#define FILTER_ACQUIRE_LOCK(_pLock, DispatchLevel)              \
    {                                                           \
        if (DispatchLevel)                                      \
        {                                                       \
            NdisDprAcquireSpinLock(_pLock);                     \
        }                                                       \
        else                                                    \
        {                                                       \
            NdisAcquireSpinLock(_pLock);                        \
        }                                                       \
    }

#define FILTER_RELEASE_LOCK(_pLock, DispatchLevel)              \
    {                                                           \
        if (DispatchLevel)                                      \
        {                                                       \
            NdisDprReleaseSpinLock(_pLock);                     \
        }                                                       \
        else                                                    \
        {                                                       \
            NdisReleaseSpinLock(_pLock);                        \
        }                                                       \
    }

#define FILTER_FREE_MEM(_pMem)    NdisFreeMemory(_pMem, 0, 0)

#define FILTER_ALLOC_MEM(_NdisHandle, _Size)     \
    NdisAllocateMemoryWithTagPriority(_NdisHandle, _Size, FILTER_ALLOC_TAG, LowPoolPriority)

const wchar_t FILTER_FRIENDLY_NAME[] = L"NDIS Sample LightWeight Filter";
// TODO: Customize this to match the service name in the INF
const wchar_t FILTER_SERVICE_NAME[] = L"NDISLWF";
// TODO: Customize this to match the GUID in the INF
const wchar_t FILTER_UNIQUE_NAME[] = L"{5cbf81bd-5055-47cd-9055-a76b2b4e3697}"; //unique name, quid name
const wchar_t LINKNAME_STRING[] = L"\\DosDevices\\NDISLWF";
const wchar_t NTDEVICE_STRING[] = L"\\Device\\NDISLWF";
constexpr UCHAR FILTER_MAJOR_NDIS_VERSION = NDIS_FILTER_MAJOR_VERSION;
constexpr UCHAR FILTER_MINOR_NDIS_VERSION = NDIS_FILTER_MINOR_VERSION;

//#define FILTER_FRIENDLY_NAME        L"NDIS Sample LightWeight Filter"
// TODO: Customize this to match the GUID in the INF
//#define FILTER_UNIQUE_NAME          L"{5cbf81bd-5055-47cd-9055-a76b2b4e3697}" //unique name, quid name
// TODO: Customize this to match the service name in the INF
//#define FILTER_SERVICE_NAME         L"NDISLWF"
//#define LINKNAME_STRING             L"\\DosDevices\\NDISLWF"
//#define NTDEVICE_STRING             L"\\Device\\NDISLWF"
//#define FILTER_MAJOR_NDIS_VERSION   NDIS_FILTER_MAJOR_VERSION
//#define FILTER_MINOR_NDIS_VERSION   NDIS_FILTER_MINOR_VERSION

typedef struct _FILTER_DEVICE_EXTENSION
{
    ULONG            Signature;
    NDIS_HANDLE      Handle;
} FILTER_DEVICE_EXTENSION, * PFILTER_DEVICE_EXTENSION;

typedef struct _QUEUE_ENTRY
{
    struct _QUEUE_ENTRY* Next;
}QUEUE_ENTRY, * PQUEUE_ENTRY;

typedef enum _FILTER_STATE
{
    FilterStateUnspecified,
    FilterInitialized,
    FilterPausing,
    FilterPaused,
    FilterRunning,
    FilterRestarting,
    FilterDetaching
} FILTER_STATE;

typedef struct _QUEUE_HEADER
{
    PQUEUE_ENTRY     Head;
    PQUEUE_ENTRY     Tail;
} QUEUE_HEADER, PQUEUE_HEADER;

typedef struct _MS_FILTER
{
    LIST_ENTRY                     FilterModuleLink;
    //Reference to this filter
    ULONG                           RefCount;

    NDIS_HANDLE                     FilterHandle;
    NDIS_STRING                     FilterModuleName;
    NDIS_STRING                     MiniportFriendlyName;
    NDIS_STRING                     MiniportName;
    NET_IFINDEX                     MiniportIfIndex;

    NDIS_STATUS                     Status;
    NDIS_EVENT                      Event;
    ULONG                           BackFillSize;
    NDIS_SPIN_LOCK                     Lock;    // Lock for protection of state and outstanding sends and recvs

    FILTER_STATE                    State;   // Which state the filter is in
    ULONG                           OutstandingSends;
    ULONG                           OutstandingRequest;
    ULONG                           OutstandingRcvs;
    NDIS_SPIN_LOCK                     SendLock;
    NDIS_SPIN_LOCK                     RcvLock;
    QUEUE_HEADER                    SendNBLQueue;
    QUEUE_HEADER                    RcvNBLQueue;


    NDIS_STRING                     FilterName;
    ULONG                           CallsRestart;
    BOOLEAN                         TrackReceives;
    BOOLEAN                         TrackSends;
#if DBG
    BOOLEAN                         bIndicating;
#endif

    PNDIS_OID_REQUEST               PendingOidRequest;

}MS_FILTER, * PMS_FILTER;

NTSTATUS
FilterDispatch(
    PDEVICE_OBJECT       DeviceObject,
    PIRP                 Irp
);

NDIS_STATUS
FilterRegisterOptions(
    NDIS_HANDLE  NdisFilterDriverHandle,
    NDIS_HANDLE  FilterDriverContext
);

NTSTATUS
FilterDeviceIoControl(
    PDEVICE_OBJECT        DeviceObject,
    PIRP                  Irp
);

_IRQL_requires_max_(PASSIVE_LEVEL)
NDIS_STATUS
FilterRegisterDevice(
    VOID
);

NDIS_STATUS
FilterSetModuleOptions(
    NDIS_HANDLE             FilterModuleContext
);

VOID FilterSendNetBufferListsComplete(
    NDIS_HANDLE         FilterModuleContext,
    PNET_BUFFER_LIST    NetBufferLists,
    ULONG               SendCompleteFlags
);

VOID
FilterSendNetBufferLists(
    NDIS_HANDLE         FilterModuleContext,
    PNET_BUFFER_LIST    NetBufferLists,
    NDIS_PORT_NUMBER    PortNumber,
    ULONG               SendFlags
);

VOID
FilterReceiveNetBufferLists(
    NDIS_HANDLE         FilterModuleContext,
    PNET_BUFFER_LIST    NetBufferLists,
    NDIS_PORT_NUMBER    PortNumber,
    ULONG               NumberOfNetBufferLists,
    ULONG               ReceiveFlags
);
