#pragma once
#include "stdafx.hpp"

static constexpr UCHAR FILTER_MAJOR_NDIS_VERSION = NDIS_FILTER_MAJOR_VERSION;
static constexpr UCHAR FILTER_MINOR_NDIS_VERSION = NDIS_FILTER_MINOR_VERSION;
static const wchar_t FILTER_FRIENDLY_NAME[] = L"NDIS Sample LightWeight Filter";
// TODO: Customize this to match the GUID in the INF
static const wchar_t FILTER_UNIQUE_NAME[] = L"{5cbf81bd-5055-47cd-9055-a76b2b4e3697}"; //unique name, quid name
// TODO: Customize this to match the service name in the INF
static const wchar_t FILTER_SERVICE_NAME[] = L"NDISLWF";
static const wchar_t LINKNAME_STRING[] = L"\\DosDevices\\NDISLWF";
static const wchar_t NTDEVICE_STRING[] = L"\\Device\\NDISLWF";
