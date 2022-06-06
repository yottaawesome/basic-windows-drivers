#include "IO.hpp"

_Use_decl_annotations_
NTSTATUS EvtWdfDriverDeviceAdd(WDFDRIVER driver, PWDFDEVICE_INIT deviceInit)
{
    UNREFERENCED_PARAMETER(driver);
    UNREFERENCED_PARAMETER(deviceInit);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"()\n"));
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
void EvtWdfDeviceFileCreate(
    WDFDEVICE Device,
    WDFREQUEST Request,
    WDFFILEOBJECT FileObject
)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(FileObject);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"()\n"));
    WdfRequestComplete(Request, 0);
}

// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfio/nc-wdfio-evt_wdf_io_queue_io_read
_Use_decl_annotations_
void EvtWdfIoQueueRead(
    WDFQUEUE queue,
    WDFREQUEST request,
    size_t length
)
{
    UNREFERENCED_PARAMETER(queue);
    UNREFERENCED_PARAMETER(length);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"()\n"));
    WdfRequestComplete(request, STATUS_SUCCESS);
}

_Use_decl_annotations_
void EvtWdfIoQueueWrite(
    WDFQUEUE queue,
    WDFREQUEST request,
    size_t length
)
{
    UNREFERENCED_PARAMETER(queue);
    UNREFERENCED_PARAMETER(length);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"()\n"));
    WdfRequestComplete(request, STATUS_SUCCESS);
}

_Use_decl_annotations_
void EvtWdfIoQueueIoDefault(
    WDFQUEUE Queue,
    WDFREQUEST Request
)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(Request);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"()\n"));
    WdfRequestComplete(Request, 0);
}

// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfio/nc-wdfio-evt_wdf_io_queue_io_device_control
_Use_decl_annotations_
void EvtIoDeviceControl(
    WDFQUEUE queue,
    WDFREQUEST request,
    size_t outputBufferLength,
    size_t inputBufferLength,
    ULONG ioControlCode
)
{
    UNREFERENCED_PARAMETER(queue);
    UNREFERENCED_PARAMETER(outputBufferLength);
    UNREFERENCED_PARAMETER(inputBufferLength);
    UNREFERENCED_PARAMETER(ioControlCode);

    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"()\n"));

    WdfRequestComplete(request, STATUS_SUCCESS);
}