module;

#include <string>
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
				UINT64& filterId
			);

		protected:
			std::shared_ptr<std::remove_pointer<HANDLE>::type> m_engineHandle; // handle for the open session to the filter engine
			bool m_addedProvider;
			bool m_addedSublayer;
			std::wstring m_sublayerName;
			std::wstring m_sublayerDescription;

			UINT64 m_filterId;
			std::wstring m_filterName;

			UINT64 m_filterId2;
			std::wstring m_filterName2;

			Callout m_outboundIPv4Callout;
			//std::wstring m_calloutName;
			//unsigned m_calloutId;

			//std::wstring m_calloutName2;
			//unsigned m_calloutId2;

			UINT64 m_contextId;
	};
}