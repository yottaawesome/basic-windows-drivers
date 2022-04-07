#include "Callouts.hpp"
#include "Identifiers.hpp"
#include "Globals.hpp"
#include "Util.hpp"

namespace ToyDriver::Callouts
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
        UNREFERENCED_PARAMETER(inMetaValues);
        UNREFERENCED_PARAMETER(layerData);
        UNREFERENCED_PARAMETER(classifyContext);
        UNREFERENCED_PARAMETER(flowContext);

        // We only inspect traffic
        classifyOut->actionType = FWP_ACTION_CONTINUE;

        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/ns-fwpsk-fwps_incoming_metadata_values0_
        // Not available at the IPV* layers: inMetaValues->packetDirection == FWP_DIRECTION_INBOUND
        switch (inFixedValues->layerId)
        {
            case FWPS_LAYER_INBOUND_IPPACKET_V4:
                //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ClassifyFn() matched process --> inbound IPv4 packet\n"));
                break;

            case FWPS_LAYER_OUTBOUND_IPPACKET_V4:
            {
                // https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/8c923f6b-ce7d-4246-a919-2424d6e1991f/process-id-from-fwpmlayeroutboundippacketv4-layer
                // Note that processId is not available at IPV* layers: https://docs.microsoft.com/en-us/windows-hardware/drivers/network/metadata-fields-at-each-filtering-layer
                // inMetaValues->currentMetadataValues & FWPS_METADATA_FIELD_PROCESS_ID)
                const size_t runningProcessId = reinterpret_cast<size_t>(PsGetCurrentProcessId());
                const DWORD processId = Util::GetProcessId(filter);
                if (!processId || !runningProcessId || processId != runningProcessId)
                    return;

                if (layerData)
                    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Layer data not null\n"));

                KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ClassifyFn() matched process --> outbound IPv4 packet\n"));
                break;
            }


            case FWPS_LAYER_INBOUND_TRANSPORT_V4:
                //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ClassifyFn() matched process --> inbound TCP\n"));
                break;

            case FWPS_LAYER_OUTBOUND_TRANSPORT_V4:
                //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ClassifyFn() matched process --> outbound TCP\n"));
                break;

            default:
                break;
        }
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
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "NotifyFn()\n"));
        return STATUS_SUCCESS;
    }

    NTSTATUS RegisterCallout(
        GUID  calloutKey,
        FWPS_CALLOUT_CLASSIFY_FN3 classifyCallout,
        FWPS_CALLOUT_NOTIFY_FN3 notifyCallout
    )
    {
        if (!g_deviceObject)
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "RegisterCallouts(): g_deviceObject is nullptr\n"));
            return STATUS_INVALID_HANDLE;
        }

        UINT32 calloutId = 0;
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/ns-fwpsk-fwps_callout2_
        // FWPS_CALLOUT3 doesn't appear to be documented, only 0-2 are
        FWPS_CALLOUT3 sCallout
        {
            .calloutKey = calloutKey,
            .classifyFn = classifyCallout,
            .notifyFn = notifyCallout
        };
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/fwpsk/nf-fwpsk-fwpscalloutregister0
        // This should actually be FwpsCalloutRegister3, but it's not documented
        NTSTATUS status = FwpsCalloutRegister3(
            g_deviceObject,
            &sCallout,
            &calloutId
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FwpsCalloutRegister3() failed %lu\n", status));
            return status;
        }

        return status;
    }

    NTSTATUS RegisterCallouts()
    {
        NTSTATUS status = RegisterCallout(
            Identifiers::WFP_OUTBOUND_IPV4_CALLOUT_GUID,
            ClassifyFn,
            NotifyFn
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering WFP_OUTBOUND_IPV4_CALLOUT_GUID failed %lu\n", status));
            return status;
        }

        status = RegisterCallout(
            Identifiers::WFP_INBOUND_IPV4_CALLOUT_GUID,
            ClassifyFn,
            NotifyFn
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering WFP_INBOUND_IPV4_CALLOUT_GUID failed %lu\n", status));
            return status;
        }

        status = RegisterCallout(
            Identifiers::WFP_OUTBOUND_TCP_GUID,
            ClassifyFn,
            NotifyFn
        );
        if (NT_ERROR(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Registering WFP_OUTBOUND_TCP_GUID failed %lu\n", status));
            return status;
        }

        return status;
    }
}