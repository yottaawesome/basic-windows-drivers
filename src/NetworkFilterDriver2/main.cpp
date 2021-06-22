#include "stdafx.hpp"

NDIS_HANDLE         NdisFilterDeviceHandle = NULL;
NDIS_HANDLE         FilterDriverHandle; // NDIS handle for filter driver
NDIS_HANDLE         FilterDriverObject;
PDEVICE_OBJECT      NdisDeviceObject = NULL;

constexpr UCHAR FILTER_MAJOR_NDIS_VERSION = NDIS_FILTER_MAJOR_VERSION;
constexpr UCHAR FILTER_MINOR_NDIS_VERSION = NDIS_FILTER_MINOR_VERSION;
const wchar_t FILTER_FRIENDLY_NAME[] = L"NDIS Sample LightWeight Filter";
// TODO: Customize this to match the GUID in the INF
const wchar_t FILTER_UNIQUE_NAME[] = L"{5cbf81bd-5055-47cd-9055-a76b2b4e3697}"; //unique name, quid name
// TODO: Customize this to match the service name in the INF
const wchar_t FILTER_SERVICE_NAME[] = L"NDISLWF";
const wchar_t LINKNAME_STRING[] = L"\\DosDevices\\NDISLWF";
const wchar_t NTDEVICE_STRING[] = L"\\Device\\NDISLWF";

NDIS_STATUS
FilterAttach(
    NDIS_HANDLE                     NdisFilterHandle,
    NDIS_HANDLE                     FilterDriverContext,
    PNDIS_FILTER_ATTACH_PARAMETERS  AttachParameters
)
{
    UNREFERENCED_PARAMETER(NdisFilterHandle);
    UNREFERENCED_PARAMETER(FilterDriverContext);
    UNREFERENCED_PARAMETER(AttachParameters);

    return STATUS_SUCCESS;
}

VOID
FilterDetach(
    NDIS_HANDLE     FilterModuleContext
)
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
}

NDIS_STATUS
FilterRestart(
    NDIS_HANDLE                     FilterModuleContext,
    PNDIS_FILTER_RESTART_PARAMETERS RestartParameters
)
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
    UNREFERENCED_PARAMETER(RestartParameters);
    return STATUS_SUCCESS;
}

typedef struct _FILTER_DEVICE_EXTENSION
{
    ULONG            Signature;
    NDIS_HANDLE      Handle;
} FILTER_DEVICE_EXTENSION, * PFILTER_DEVICE_EXTENSION;

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

NDIS_STATUS
FilterPause(
    NDIS_HANDLE                     FilterModuleContext,
    PNDIS_FILTER_PAUSE_PARAMETERS   PauseParameters
)
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
    UNREFERENCED_PARAMETER(PauseParameters);

    return STATUS_SUCCESS;
}

NTSTATUS
FilterDispatch(
    PDEVICE_OBJECT       DeviceObject,
    PIRP                 Irp
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);


    PIO_STACK_LOCATION       IrpStack;
    NTSTATUS                 Status = STATUS_SUCCESS;


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

NTSTATUS
FilterDeviceIoControl(
    PDEVICE_OBJECT        DeviceObject,
    PIRP                  Irp
)
{
    PIO_STACK_LOCATION          IrpSp;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PFILTER_DEVICE_EXTENSION    FilterDeviceExtension;
    PUCHAR                      InputBuffer;
    PUCHAR                      OutputBuffer;
    ULONG                       InputBufferLength, OutputBufferLength;
    PLIST_ENTRY                 Link;
    PUCHAR                      pInfo;
    ULONG                       InfoLength = 0;
    //PMS_FILTER                  pFilter = NULL;
    //BOOLEAN                     bFalse = FALSE;


    UNREFERENCED_PARAMETER(DeviceObject);


    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    if (IrpSp->FileObject == NULL)
    {
        return(STATUS_UNSUCCESSFUL);
    }


    FilterDeviceExtension = (PFILTER_DEVICE_EXTENSION)NdisGetDeviceReservedExtension(DeviceObject);

    ASSERT(FilterDeviceExtension->Signature == 'FTDR');

    Irp->IoStatus.Information = 0;

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode)
    {
    case 1:
        break;
    default:
        break;
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = InfoLength;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;


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
        FilterDeviceExtension = (PFILTER_DEVICE_EXTENSION)NdisGetDeviceReservedExtension(NdisDeviceObject);
        FilterDeviceExtension->Signature = 'FTDR';
        FilterDeviceExtension->Handle = FilterDriverHandle;
    }

    //DEBUGP(DL_TRACE, "<==FilterRegisterDevice: %x\n", Status);

    return (Status);

}

void DriverUnload(_In_ PDRIVER_OBJECT driverObject)
{
	UNREFERENCED_PARAMETER(driverObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DriverUnload()\n"));

	FilterDeregisterDevice();
	NdisFDeregisterFilterDriver(FilterDriverHandle);
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
        FChars.AttachHandler = FilterAttach;
        FChars.DetachHandler = FilterDetach;
        FChars.RestartHandler = FilterRestart;
        FChars.PauseHandler = FilterPause;
        
        FChars.SetFilterModuleOptionsHandler = nullptr;
        FChars.SetOptionsHandler = nullptr;
        FChars.OidRequestHandler = nullptr;
        FChars.OidRequestCompleteHandler = nullptr;
        FChars.CancelOidRequestHandler = nullptr;

        FChars.SendNetBufferListsHandler = nullptr;
        FChars.ReturnNetBufferListsHandler = nullptr;
        FChars.SendNetBufferListsCompleteHandler = nullptr;
        FChars.ReceiveNetBufferListsHandler = nullptr;
        FChars.DevicePnPEventNotifyHandler = nullptr;
        FChars.NetPnPEventHandler = nullptr;
        FChars.StatusHandler = nullptr;
        FChars.CancelSendNetBufferListsHandler = nullptr;

        driverObject->DriverUnload = DriverUnload;

        FilterDriverHandle = NULL;

        Status = NdisFRegisterFilterDriver(
            driverObject,
            (NDIS_HANDLE)FilterDriverObject,
            &FChars,
            &FilterDriverHandle
        );

        if (Status != NDIS_STATUS_SUCCESS)
        {
            //DEBUGP(DL_WARN, "Register filter driver failed.\n");
            break;
        }

        Status = FilterRegisterDevice();

        if (Status != NDIS_STATUS_SUCCESS)
        {
            NdisFDeregisterFilterDriver(FilterDriverHandle);
            //FILTER_FREE_LOCK(&FilterListLock);
            //DEBUGP(DL_WARN, "Register device for the filter driver failed.\n");
            break;
        }
    } while (bFalse);

    //DEBUGP(DL_TRACE, "<===DriverEntry, Status = %8x\n", Status);
    return Status;
}