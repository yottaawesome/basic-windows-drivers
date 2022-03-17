#pragma warning(disable:4201)
#pragma warning(disable:4471)
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
#include <Wdmsec.h>
#include <guiddef.h>
#include <ndis.h>
#include <Fwpsk.h>

// {2CC08215-0458-4B8B-9B03-1A00F4914599}
DEFINE_GUID(WFP_TEST_CALLOUT,
	0x2cc08215, 0x458, 0x4b8b, 0x9b, 0x3, 0x1a, 0x0, 0xf4, 0x91, 0x45, 0x99);

extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT  DriverObject,
	_In_ PUNICODE_STRING RegistryPath
);

// FWPS_CALLOUT_CLASSIFY_FN3
void ClassifyFn(
    _In_ const FWPS_INCOMING_VALUES0* inFixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
    _Inout_opt_ void* layerData,
    _In_opt_ const void* classifyContext,
    _In_ const FWPS_FILTER3* filter,
    _In_ UINT64 flowContext,
    _Inout_ FWPS_CLASSIFY_OUT0* classifyOut
);

NTSTATUS NotifyFn(
    _In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
    _In_ const GUID* filterKey,
    _Inout_ FWPS_FILTER3* filter
);

void DriverUnload(_In_ WDFDRIVER DriverObject);
