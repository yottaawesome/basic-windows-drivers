#include "stdafx.hpp"
#include "FilterCallbacks.hpp"

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

_Use_decl_annotations_
void FilterDetach(
    _In_ NDIS_HANDLE     FilterModuleContext
)
{
    UNREFERENCED_PARAMETER(FilterModuleContext);
}

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