#pragma once
#include "Headers.hpp"

namespace ToyDriver::Callouts
{
    NTSTATUS RegisterAllCallouts();
    NTSTATUS UnregisterAllCallouts();

    namespace OutboundTCP
    {   
        // {06A7CEA1-7826-4330-B2F9-4D801BDC0B37}
        static const GUID Key =
        { 0x6a7cea1, 0x7826, 0x4330, { 0xb2, 0xf9, 0x4d, 0x80, 0x1b, 0xdc, 0xb, 0x37 } };

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
    }

    // FWPS_CALLOUT_CLASSIFY_FN3
    namespace InboundIPv4
    {
        // {B0440EFA-87CC-4727-8D95-972FC02FA4CA}
        static const GUID Key =
        { 0xb0440efa, 0x87cc, 0x4727, { 0x8d, 0x95, 0x97, 0x2f, 0xc0, 0x2f, 0xa4, 0xca } };

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
    }
    
    namespace OutboundIPv4
    {
        // {2CC08215-0458-4B8B-9B03-1A00F4914599}
        static const GUID Key =
        { 0x2cc08215, 0x458, 0x4b8b, { 0x9b, 0x3, 0x1a, 0x0, 0xf4, 0x91, 0x45, 0x99 } };

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
    }

    namespace OutboundICMPError
    {
        // {EB79F29B-0CCB-48D7-8A33-45CB311E7125}
        static const GUID Key =
        { 0xeb79f29b, 0xccb, 0x48d7, { 0x8a, 0x33, 0x45, 0xcb, 0x31, 0x1e, 0x71, 0x25 } };

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
    }

    namespace InboundICMPError
    {
        // {56E4C747-B3F4-464F-89CC-511D1999B27B}
        static const GUID Key =
        { 0x56e4c747, 0xb3f4, 0x464f, { 0x89, 0xcc, 0x51, 0x1d, 0x19, 0x99, 0xb2, 0x7b } };

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
    }
}