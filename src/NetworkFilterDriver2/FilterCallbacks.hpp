#pragma once
#include "stdafx.hpp"

// For a breakdown of filter states, see:
// https://docs.microsoft.com/en-us/windows-hardware/drivers/network/filter-module-states-and-operations

NDIS_STATUS FilterAttach(
    _In_ NDIS_HANDLE                     NdisFilterHandle,
    _In_ NDIS_HANDLE                     FilterDriverContext,
    _In_ PNDIS_FILTER_ATTACH_PARAMETERS  AttachParameters
);

void FilterDetach(
    _In_ NDIS_HANDLE     FilterModuleContext
);

NDIS_STATUS FilterRestart(
    _In_ NDIS_HANDLE                     FilterModuleContext,
    _In_ PNDIS_FILTER_RESTART_PARAMETERS RestartParameters
);

NDIS_STATUS FilterPause(
    _In_ NDIS_HANDLE                     FilterModuleContext,
    _In_ PNDIS_FILTER_PAUSE_PARAMETERS   PauseParameters
);
