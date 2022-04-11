module;

#include <string>
#include <vector>
#include <memory>
#include <format>
#include <Windows.h>
#include <fwpmu.h>

export module wfpcontroller.wfp;
import wfpcontroller.wfp.callout;

export namespace WFPController::WFP
{
	export class WFPEngine
	{
		public:
			virtual ~WFPEngine();
			WFPEngine();

		public:
			virtual void Close();
			virtual void OpenFilterEngine();
			virtual void RegisterProvider();
			virtual void AddContext();
			virtual void AddCallouts();
			virtual void AddOutboundTCPPacketCallout();
			virtual void AddOutboundIPv4PacketCallout();
			virtual void AddInboundIPv4PacketCallout();
			virtual void AddInboundICMPErrorCallout();
			virtual void AddOutboundICMPErrorCallout();
			virtual void AddSublayer();
			virtual void AddFilters();

		protected:
			virtual void AddFilter(
				const std::wstring& filterName,
				const GUID& layerKey,
				const GUID& sublayerGuid,
				const GUID& calloutKey,
				const FWP_DATA_TYPE weightType,
				const FWP_ACTION_TYPE actionType,
				const std::vector<FWPM_FILTER_CONDITION0>& conditions,
				UINT64& filterId
			);

		protected:
			std::shared_ptr<std::remove_pointer<HANDLE>::type> m_engineHandle; // handle for the open session to the filter engine
			bool m_addedProvider;
			bool m_addedSublayer;
			std::wstring m_sublayerName;
			std::wstring m_sublayerDescription;

			// Need to split these out into object; too confusing to track everything here
			UINT64 m_outboundIPv4FilterID;
			std::wstring m_outboundIPv4FilterName;

			UINT64 m_inboundICMPErrorFilterID;
			std::wstring m_inboundICMPErrorFilterName;

			UINT64 m_outboundICMPErrorFilterID;
			std::wstring m_outboundICMPErrorFilterName;

			UINT64 m_outboundTransportFilterID;
			std::wstring m_outboundTransportFilterName;

			Callout m_outboundIPv4Callout;
			Callout m_inboundICMPErrorCallout;
			Callout m_outboundICMPErrorCallout;
			Callout m_outboundTCPCallout;

			UINT64 m_contextId;
	};
}