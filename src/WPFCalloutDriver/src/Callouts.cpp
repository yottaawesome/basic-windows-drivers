#include "Callouts.hpp"
#include "Globals.hpp"
#include "Util.hpp"

namespace ToyDriver::Callouts
{
    UINT16 matchedPort = 0;

    NTSTATUS UnregisterAllCallouts()
    {
        NTSTATUS status = FwpsCalloutUnregisterByKey0(&Outbound::IPv4::Key);
        if (NT_ERROR(status))
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Failed unregistering OutboundIPv4: %lu\n", status));

        status = FwpsCalloutUnregisterByKey0(&Inbound::ICMPError::Key);
        if (NT_ERROR(status))
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Failed unregistering InboundICMPError: %lu\n", status));

        status = FwpsCalloutUnregisterByKey0(&Outbound::ICMPError::Key);
        if (NT_ERROR(status))
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Failed unregistering OutboundICMPError: %lu\n", status));

        status = FwpsCalloutUnregisterByKey0(&Outbound::TCP::Key);
        if (NT_ERROR(status))
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Failed unregistering OutboundTCP: %lu\n", status));

        status = FwpsCalloutUnregisterByKey0(&Inbound::TCP::Key);
        if (NT_ERROR(status))
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Failed unregistering InboundTCP: %lu\n", status));

        return STATUS_SUCCESS;
    }

    NTSTATUS RegisterCallout(
        GUID  calloutKey,
        FWPS_CALLOUT_CLASSIFY_FN3 classifyCallout,
        FWPS_CALLOUT_NOTIFY_FN3 notifyCallout
    )
    {
        if (!Globals::DriverDeviceObject)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "RegisterCallouts(): Globals::DriverDeviceObject is nullptr\n"));
            return STATUS_INVALID_HANDLE;
        }

        UINT32 calloutId = 0;
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/ns-fwpsk-fwps_callout2_
        // FWPS_CALLOUT3 doesn't appear to be documented, only 0-2 are
        FWPS_CALLOUT3 calloutInfo
        {
            .calloutKey = calloutKey,
            .classifyFn = classifyCallout,
            .notifyFn = notifyCallout
        };
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/nf-fwpsk-fwpscalloutregister0
        // This should actually be FwpsCalloutRegister3, but it's not documented
        const NTSTATUS status = FwpsCalloutRegister3(
            Globals::DriverDeviceObject,
            &calloutInfo,
            &calloutId
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FwpsCalloutRegister3() failed %lu\n", status));
            return status;
        }

        return status;
    }

    NTSTATUS RegisterAllCallouts()
    {
        NTSTATUS status = RegisterCallout(
            Outbound::IPv4::Key,
            Outbound::IPv4::ClassifyFn,
            Outbound::IPv4::NotifyFn
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering OutboundIPv4 failed %lu\n", status));
            return status;
        }

        status = RegisterCallout(
            Inbound::ICMPError::Key,
            Inbound::ICMPError::ClassifyFn,
            Inbound::ICMPError::NotifyFn
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering InboundICMP failed %lu\n", status));
            return status;
        }

        status = RegisterCallout(
            Outbound::ICMPError::Key,
            Outbound::ICMPError::ClassifyFn,
            Outbound::ICMPError::NotifyFn
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering OutboundICMP failed %lu\n", status));
            return status;
        }

        status = RegisterCallout(
            Outbound::TCP::Key,
            Outbound::TCP::ClassifyFn,
            Outbound::TCP::NotifyFn
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering OutboundTCP failed %lu\n", status));
            return status;
        }

        status = RegisterCallout(
            Inbound::TCP::Key,
            Inbound::TCP::ClassifyFn,
            Inbound::TCP::NotifyFn
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering InboundTCP failed %lu\n", status));
            return status;
        }

        return status;
    }
}

namespace ToyDriver::Callouts::Inbound::TCP
{
    void ClassifyFn(
        _In_ const FWPS_INCOMING_VALUES0* inFixedValues,
        _In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
        _Inout_opt_ void* layerData,
        _In_opt_ const void* classifyContext,
        _In_ const FWPS_FILTER3* filter,
        _In_ UINT64 flowContext,
        _Inout_ FWPS_CLASSIFY_OUT0* classifyOut
    )
    {
        UNREFERENCED_PARAMETER(inFixedValues);
        UNREFERENCED_PARAMETER(inMetaValues);
        UNREFERENCED_PARAMETER(layerData);
        UNREFERENCED_PARAMETER(classifyContext);
        UNREFERENCED_PARAMETER(filter);
        UNREFERENCED_PARAMETER(flowContext);

        // We only inspect traffic
        classifyOut->actionType = FWP_ACTION_CONTINUE;

        const UINT16 localPort =
            inFixedValues->incomingValue[FWPS_FIELD_INBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value.uint16;
        if (matchedPort && localPort && localPort == matchedPort)
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"(): matched on port %hu\n", localPort));
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"(): inbound transport %hu\n", localPort));
    }

    NTSTATUS NotifyFn(
        _In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
        _In_ const GUID* filterKey,
        _Inout_ FWPS_FILTER3* filter
    )
    {
        UNREFERENCED_PARAMETER(notifyType);
        UNREFERENCED_PARAMETER(filterKey);
        UNREFERENCED_PARAMETER(filter);

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"(): invoked\n"));

        return STATUS_SUCCESS;
    }
}

namespace ToyDriver::Callouts::Outbound::TCP
{
    void ClassifyFn(
        _In_ const FWPS_INCOMING_VALUES0* inFixedValues,
        _In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
        _Inout_opt_ void* layerData,
        _In_opt_ const void* classifyContext,
        _In_ const FWPS_FILTER3* filter,
        _In_ UINT64 flowContext,
        _Inout_ FWPS_CLASSIFY_OUT0* classifyOut
    )
    {
        UNREFERENCED_PARAMETER(inFixedValues);
        UNREFERENCED_PARAMETER(inMetaValues);
        UNREFERENCED_PARAMETER(layerData);
        UNREFERENCED_PARAMETER(classifyContext);
        UNREFERENCED_PARAMETER(filter);
        UNREFERENCED_PARAMETER(flowContext);

        // We only inspect traffic
        classifyOut->actionType = FWP_ACTION_CONTINUE;

        const size_t runningProcessId = reinterpret_cast<size_t>(PsGetCurrentProcessId());
        const DWORD processId = Util::GetProcessId(filter);
        if (!processId || !runningProcessId || processId != runningProcessId)
            return;

        const UINT16 localPort =
            inFixedValues->incomingValue[FWPS_FIELD_OUTBOUND_TRANSPORT_V4_IP_LOCAL_PORT].value.uint16;
        matchedPort = localPort;
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"(): outbound transport %hu\n", localPort));
    }

    NTSTATUS NotifyFn(
        _In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
        _In_ const GUID* filterKey,
        _Inout_ FWPS_FILTER3* filter
    )
    {
        UNREFERENCED_PARAMETER(notifyType);
        UNREFERENCED_PARAMETER(filterKey);
        UNREFERENCED_PARAMETER(filter);
        
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"(): invoked\n"));

        return STATUS_SUCCESS;
    }
}

namespace ToyDriver::Callouts::Inbound::IPv4
{
    _Use_decl_annotations_
    void ClassifyFn(
        const FWPS_INCOMING_VALUES0* inFixedValues,
        const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
        void* layerData,
        const void* classifyContext,
        const FWPS_FILTER3* filter,
        UINT64 flowContext,
        FWPS_CLASSIFY_OUT0* classifyOut
    )
    {
        UNREFERENCED_PARAMETER(inFixedValues);
        UNREFERENCED_PARAMETER(inMetaValues);
        UNREFERENCED_PARAMETER(layerData);
        UNREFERENCED_PARAMETER(classifyContext);
        UNREFERENCED_PARAMETER(filter);
        UNREFERENCED_PARAMETER(flowContext);

        // We only inspect traffic
        classifyOut->actionType = FWP_ACTION_CONTINUE;

        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/ns-fwpsk-fwps_incoming_metadata_values0_
        // Not available at the IPV* layers: inMetaValues->packetDirection == FWP_DIRECTION_INBOUND
        // inFixedValues->layerId
    }

    NTSTATUS NotifyFn(
        _In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
        _In_ const GUID* filterKey,
        _Inout_ FWPS_FILTER3* filter
    )
    {
        UNREFERENCED_PARAMETER(notifyType);
        UNREFERENCED_PARAMETER(filterKey);
        UNREFERENCED_PARAMETER(filter);

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"(): invoked\n"));
        
        return STATUS_SUCCESS;
    }
}

namespace ToyDriver::Callouts::Outbound::ICMPError
{
    void ClassifyFn(
        _In_ const FWPS_INCOMING_VALUES0* inFixedValues,
        _In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
        _Inout_opt_ void* layerData,
        _In_opt_ const void* classifyContext,
        _In_ const FWPS_FILTER3* filter,
        _In_ UINT64 flowContext,
        _Inout_ FWPS_CLASSIFY_OUT0* classifyOut
    )
    {
        UNREFERENCED_PARAMETER(inFixedValues);
        UNREFERENCED_PARAMETER(inMetaValues);
        UNREFERENCED_PARAMETER(layerData);
        UNREFERENCED_PARAMETER(classifyContext);
        UNREFERENCED_PARAMETER(filter);
        UNREFERENCED_PARAMETER(flowContext);

        // We only inspect traffic
        classifyOut->actionType = FWP_ACTION_CONTINUE;

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"(): ICMP outbound error detected\n"));
    }

    _Use_decl_annotations_
        NTSTATUS NotifyFn(
            FWPS_CALLOUT_NOTIFY_TYPE notifyType,
            const GUID* filterKey,
            FWPS_FILTER3* filter
        )
    {
        UNREFERENCED_PARAMETER(notifyType);
        UNREFERENCED_PARAMETER(filterKey);
        UNREFERENCED_PARAMETER(filter);
        
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__ "(): invoked\n"));

        return STATUS_SUCCESS;
    }
}

namespace ToyDriver::Callouts::Inbound::ICMPError
{
    void ClassifyFn(
        _In_ const FWPS_INCOMING_VALUES0* inFixedValues,
        _In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
        _Inout_opt_ void* layerData,
        _In_opt_ const void* classifyContext,
        _In_ const FWPS_FILTER3* filter,
        _In_ UINT64 flowContext,
        _Inout_ FWPS_CLASSIFY_OUT0* classifyOut
    )
    {
        UNREFERENCED_PARAMETER(inFixedValues);
        UNREFERENCED_PARAMETER(inMetaValues);
        UNREFERENCED_PARAMETER(layerData);
        UNREFERENCED_PARAMETER(classifyContext);
        UNREFERENCED_PARAMETER(filter);
        UNREFERENCED_PARAMETER(flowContext);

        // We only inspect traffic
        classifyOut->actionType = FWP_ACTION_CONTINUE;

        // See https://docs.microsoft.com/en-us/windows-hardware/drivers/network/data-offset-positions

        // Not available
        //const size_t runningProcessId = reinterpret_cast<size_t>(PsGetCurrentProcessId());
        const UINT32 remoteIpAddress =
            inFixedValues->incomingValue[FWPS_FIELD_INBOUND_ICMP_ERROR_V4_IP_REMOTE_ADDRESS].value.uint32;
        const UINT16 embeddedLocalPort = 
            inFixedValues->incomingValue[FWPS_FIELD_INBOUND_ICMP_ERROR_V4_EMBEDDED_LOCAL_PORT].value.uint16;
        const UINT8 icmpType = 
            inFixedValues->incomingValue[FWPS_FIELD_INBOUND_ICMP_ERROR_V4_ICMP_TYPE].value.uint8;
        const UINT8 icmpCode = 
            inFixedValues->incomingValue[FWPS_FIELD_INBOUND_ICMP_ERROR_V4_ICMP_CODE].value.uint8;

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__": ICMP inbound error detected, remote address %lu, embedded local port %hu type %hu, code %hu\n", remoteIpAddress, embeddedLocalPort, icmpType, icmpCode));
    }

    _Use_decl_annotations_
    NTSTATUS NotifyFn(
        FWPS_CALLOUT_NOTIFY_TYPE notifyType,
        const GUID* filterKey,
        FWPS_FILTER3* filter
    )
    {
        UNREFERENCED_PARAMETER(notifyType);
        UNREFERENCED_PARAMETER(filterKey);
        UNREFERENCED_PARAMETER(filter);
        
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__ "(): invoked\n"));

        return STATUS_SUCCESS;
    }
}

namespace ToyDriver::Callouts::Outbound::IPv4
{
    _Use_decl_annotations_
    void ClassifyFn(
        _In_ const FWPS_INCOMING_VALUES0* inFixedValues,
        _In_ const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
        _Inout_opt_ void* layerData,
        _In_opt_ const void* classifyContext,
        _In_ const FWPS_FILTER3* filter,
        _In_ UINT64 flowContext,
        _Inout_ FWPS_CLASSIFY_OUT0* classifyOut
    )
    {
        UNREFERENCED_PARAMETER(inFixedValues);
        UNREFERENCED_PARAMETER(inMetaValues);
        UNREFERENCED_PARAMETER(layerData);
        UNREFERENCED_PARAMETER(classifyContext);
        UNREFERENCED_PARAMETER(flowContext);

        // We only inspect traffic
        classifyOut->actionType = FWP_ACTION_CONTINUE;

        // https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/8c923f6b-ce7d-4246-a919-2424d6e1991f/process-id-from-fwpmlayeroutboundippacketv4-layer
        // Note that processId is not available at IPV* layers: https://docs.microsoft.com/en-us/windows-hardware/drivers/network/metadata-fields-at-each-filtering-layer
        // inMetaValues->currentMetadataValues & FWPS_METADATA_FIELD_PROCESS_ID)
        const size_t runningProcessId = reinterpret_cast<size_t>(PsGetCurrentProcessId());
        const DWORD processId = Util::GetProcessId(filter);
        if (!processId || !runningProcessId || processId != runningProcessId)
            return;

        if (layerData)
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Layer data not null\n"));

        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"(): matched process --> outbound IPv4 packet\n"));
    }

    _Use_decl_annotations_
    NTSTATUS NotifyFn(
        FWPS_CALLOUT_NOTIFY_TYPE notifyType,
        const GUID* filterKey,
        FWPS_FILTER3* filter
    )
    {
        UNREFERENCED_PARAMETER(notifyType);
        UNREFERENCED_PARAMETER(filterKey);
        UNREFERENCED_PARAMETER(filter);
        
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __FUNCTION__"(): invoked\n"));

        return STATUS_SUCCESS;
    }
}
