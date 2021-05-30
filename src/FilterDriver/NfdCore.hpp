#pragma once
#pragma warning(disable:4201)  //nonstandard extension used : nameless struct/union
//#include <ntddk.h>
#include <ndis.h>

const wchar_t* FILTER_FRIENDLY_NAME = L"NDIS Sample LightWeight Filter";
// TODO: Customize this to match the service name in the INF
const wchar_t* FILTER_SERVICE_NAME = L"NDISLWF";
// TODO: Customize this to match the GUID in the INF
const wchar_t* FILTER_UNIQUE_NAME = L"{5cbf81bd-5055-47cd-9055-a76b2b4e3697}"; //unique name, quid name

constexpr UCHAR FILTER_MAJOR_NDIS_VERSION = NDIS_FILTER_MAJOR_VERSION;
constexpr UCHAR FILTER_MINOR_NDIS_VERSION = NDIS_FILTER_MINOR_VERSION;
const wchar_t* LINKNAME_STRING = L"\\DosDevices\\NDISLWF";
const wchar_t* NTDEVICE_STRING = L"\\Device\\NDISLWF";

typedef struct _FILTER_DEVICE_EXTENSION
{
    ULONG            Signature;
    NDIS_HANDLE      Handle;
} FILTER_DEVICE_EXTENSION, * PFILTER_DEVICE_EXTENSION;