#pragma once
#include "stdafx.hpp"

static NDIS_HANDLE         NdisFilterDeviceHandle = nullptr;
static NDIS_HANDLE         FilterDriverHandle; // NDIS handle for filter driver
static NDIS_HANDLE         FilterDriverObject;
static PDEVICE_OBJECT      NdisDeviceObject = nullptr;
