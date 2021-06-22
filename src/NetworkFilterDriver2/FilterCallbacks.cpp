#include "stdafx.hpp"
#include "FilterCallbacks.hpp"

// https://docs.microsoft.com/en-us/windows-hardware/drivers/network/attaching-a-filter-module
// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ndis/nc-ndis-filter_attach
_Use_decl_annotations_
NDIS_STATUS FilterAttach(
    _In_ NDIS_HANDLE                     NdisFilterHandle,
    _In_ NDIS_HANDLE                     FilterDriverContext,
    _In_ PNDIS_FILTER_ATTACH_PARAMETERS  AttachParameters
)
{
    UNREFERENCED_PARAMETER(NdisFilterHandle);
    UNREFERENCED_PARAMETER(FilterDriverContext);
    UNREFERENCED_PARAMETER(AttachParameters);

    return STATUS_SUCCESS;
}

// https://docs.microsoft.com/en-us/windows-hardware/drivers/network/detaching-a-filter-module
// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ndis/nc-ndis-filter_detach
_Use_decl_annotations_
void FilterDetach(
    _In_ NDIS_HANDLE     FilterModuleContext
)
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
}

// https://docs.microsoft.com/en-us/windows-hardware/drivers/network/starting-and-pausing-a-filter-module
// https://docs.microsoft.com/en-us/windows-hardware/drivers/network/starting-a-filter-module
// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ndis/nc-ndis-filter_restart
_Use_decl_annotations_
NDIS_STATUS FilterRestart(
    _In_ NDIS_HANDLE                     FilterModuleContext,
    _In_ PNDIS_FILTER_RESTART_PARAMETERS RestartParameters
)
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
    UNREFERENCED_PARAMETER(RestartParameters);
    return STATUS_SUCCESS;
}

// https://docs.microsoft.com/en-us/windows-hardware/drivers/network/starting-and-pausing-a-filter-module
// https://docs.microsoft.com/en-us/windows-hardware/drivers/network/pausing-a-filter-module
// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ndis/nc-ndis-filter_pause
_Use_decl_annotations_
NDIS_STATUS FilterPause(
    _In_ NDIS_HANDLE                     FilterModuleContext,
    _In_ PNDIS_FILTER_PAUSE_PARAMETERS   PauseParameters
)
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
    UNREFERENCED_PARAMETER(PauseParameters);

    return STATUS_SUCCESS;
}
