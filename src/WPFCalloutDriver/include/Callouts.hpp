#pragma once
#include "Headers.hpp"

namespace ToyDriver::Callouts
{
    // FWPS_CALLOUT_CLASSIFY_FN3
    void InboundIPv4ClassifyFn(
        _In_ const FWPS_INCOMING_VALUES0* inFixedValues,
        _In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
        _Inout_opt_ void* layerData,
        _In_opt_ const void* classifyContext,
        _In_ const FWPS_FILTER3* filter,
        _In_ UINT64 flowContext,
        _Inout_ FWPS_CLASSIFY_OUT0* classifyOut
    );

    void OutboundIPv4ClassifyFn(
        _In_ const FWPS_INCOMING_VALUES0* inFixedValues,
        _In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
        _Inout_opt_ void* layerData,
        _In_opt_ const void* classifyContext,
        _In_ const FWPS_FILTER3* filter,
        _In_ UINT64 flowContext,
        _Inout_ FWPS_CLASSIFY_OUT0* classifyOut
    );
    NTSTATUS OutboundIPv4NotifyFn(
        _In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
        _In_ const GUID* filterKey,
        _Inout_ FWPS_FILTER3* filter
    );

    void InboundICMPErrorClassifyFn(
        _In_ const FWPS_INCOMING_VALUES0* inFixedValues,
        _In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
        _Inout_opt_ void* layerData,
        _In_opt_ const void* classifyContext,
        _In_ const FWPS_FILTER3* filter,
        _In_ UINT64 flowContext,
        _Inout_ FWPS_CLASSIFY_OUT0* classifyOut
    );
    NTSTATUS InboundICMPErrorNotifyFn(
        _In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
        _In_ const GUID* filterKey,
        _Inout_ FWPS_FILTER3* filter
    );

    NTSTATUS RegisterCallouts();
    NTSTATUS UnregisterCallouts();
}