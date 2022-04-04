#pragma warning(disable:4201)
#pragma warning(disable:4471)
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
#include <Wdmsec.h>
#include <guiddef.h>
#include <ndis.h>
#include <Fwpsk.h>
#include <fwpmtypes.h>
#include <fwptypes.h>
#include <fwpmk.h>

// {2CC08215-0458-4B8B-9B03-1A00F4914599}
static const GUID WFP_OUTBOUND_IPV4_CALLOUT_GUID =
    { 0x2cc08215, 0x458, 0x4b8b, { 0x9b, 0x3, 0x1a, 0x0, 0xf4, 0x91, 0x45, 0x99 }};

// {B0440EFA-87CC-4727-8D95-972FC02FA4CA}
static const GUID WFP_INBOUND_IPV4_CALLOUT_GUID =
    { 0xb0440efa, 0x87cc, 0x4727, { 0x8d, 0x95, 0x97, 0x2f, 0xc0, 0x2f, 0xa4, 0xca } };

// {06A7CEA1-7826-4330-B2F9-4D801BDC0B37}
static const GUID WFP_OUTBOUND_TCP_GUID =
    { 0x6a7cea1, 0x7826, 0x4330, { 0xb2, 0xf9, 0x4d, 0x80, 0x1b, 0xdc, 0xb, 0x37 } };

// {270B7DD9-17E7-480E-B25F-04BB81F42495}
static const GUID WFP_PROVIDER_GUID =
    { 0x270b7dd9, 0x17e7, 0x480e, { 0xb2, 0x5f, 0x4, 0xbb, 0x81, 0xf4, 0x24, 0x95 }};

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
