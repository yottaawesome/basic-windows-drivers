#pragma once
#include <ntddk.h>
#include <wdf.h>

// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/wdfdriver/nc-wdfdriver-evt_wdf_driver_device_add
NTSTATUS EvtWdfDriverDeviceAdd(
    _In_ WDFDRIVER driver, 
    _Inout_ PWDFDEVICE_INIT deviceInit
);
void EvtWdfDeviceFileCreate(
    _In_ WDFDEVICE device,
    _In_ WDFREQUEST request,
    _In_ WDFFILEOBJECT fileObject
);
void EvtWdfIoQueueRead(
    _In_ WDFQUEUE queue,
    _In_ WDFREQUEST request,
    _In_ size_t length
);
void EvtWdfIoQueueWrite(
    _In_ WDFQUEUE queue,
    _In_ WDFREQUEST request,
    _In_ size_t length
);
void EvtWdfIoQueueIoDefault(
    _In_ WDFQUEUE queue,
    _In_ WDFREQUEST request
);
void EvtIoDeviceControl(
    _In_ WDFQUEUE queue,
    _In_ WDFREQUEST request,
    _In_ size_t outputBufferLength,
    _In_ size_t inputBufferLength,
    _In_ ULONG ioControlCode
);
