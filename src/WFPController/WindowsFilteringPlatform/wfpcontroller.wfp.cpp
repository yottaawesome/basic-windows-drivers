module;

#include <stdexcept>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <format>
#include <source_location>
#include <Windows.h>
#include <fwpmu.h>

module wfpcontroller.wfp;
import wfpcontroller.identifiers;
import wfpcontroller.wfp.callout;
import wfpcontroller.wfp.wfperror;
import wfpcontroller.error.functions;

namespace WFPController::WFP
{
	WFPEngine::~WFPEngine()
	{
		Close();
	}

	WFPEngine::WFPEngine()
		: m_addedProvider(false),
		m_addedSublayer(false),
		m_engineHandle(nullptr),
		m_outboundIPv4FilterName(L"IPv4 outbound packet"),
		m_outboundIPv4FilterID(0),
		m_inboundICMPErrorFilterName(L"Inbound ICMP error file"),
		m_inboundICMPErrorFilterID(0),
		m_sublayerName(L"ToyCalloutDriverSublayer"),
		m_sublayerDescription(L"Just a toy sublayer"),
		m_contextId(0),
		m_outboundICMPErrorFilterID(0),
		m_outboundICMPErrorFilterName(L"Outbound ICMP error filter"),
		m_outboundTransportFilterID(0),
		m_outboundTransportFilterName(L"Outbound transport filter")
	{
	}

	void WFPEngine::RegisterProvider()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		DWORD data = 321;
		std::wstring providerName = L"provider name";
		const FWPM_PROVIDER0  provider = {
			.providerKey = Identifiers::ProviderKey,
			.displayData = {.name = providerName.data()},
			.providerData = {
				.size = sizeof(data),
				.data = reinterpret_cast<UINT8*>(&data),
			}
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmprovideradd0
		const DWORD status = FwpmProviderAdd0(
			m_engineHandle.get(),
			&provider,
			nullptr
		);
		if (status != ERROR_SUCCESS)
			throw WFPError(std::source_location::current(), "Failed to add provider", status);
		m_addedProvider = true;
	}

	void WFPEngine::Close()
	{
		if (!m_engineHandle)
			return;
		
		m_outboundIPv4Callout.Remove();
		m_inboundICMPErrorCallout.Remove();
		m_outboundICMPErrorCallout.Remove();
		m_outboundTCPCallout.Remove();
		m_engineHandle = nullptr;
	}

	void WFPEngine::AddContext()
	{
		std::wstring contextName = L"TODO: find a better name for this context";
		DWORD currentProcessId = GetCurrentProcessId();
		FWP_BYTE_BLOB byteBlob = { 
			.size = sizeof(currentProcessId),
			.data = reinterpret_cast<UINT8*>(&currentProcessId)
		};
		FWPM_PROVIDER_CONTEXT3 providerContext = {
			.providerContextKey = Identifiers::ProviderContextKey,
			.displayData = {.name = contextName.data()},
			.providerKey = (GUID*)&Identifiers::ProviderKey,
			.type = FWPM_GENERAL_CONTEXT,
			.dataBuffer = &byteBlob,
		};
		//UuidCreate(&(providerContext.providerContextKey));
		const DWORD status = FwpmProviderContextAdd3(m_engineHandle.get(), &providerContext, nullptr, &m_contextId);
		if (status != ERROR_SUCCESS)
			throw WFPError(std::source_location::current(), "FwpmProviderContextAdd3() failed", status);
	}

	void WFPEngine::AddOutboundTCPPacketCallout()
	{
		m_outboundTCPCallout = Callout(
			m_engineHandle,
			Identifiers::OutboundTCPCalloutKey,
			Identifiers::ProviderKey,
			FWPM_LAYER_OUTBOUND_TRANSPORT_V4,
			FWPM_CALLOUT_FLAG_USES_PROVIDER_CONTEXT,
			L"Outbound TCP Callout"
		);
		m_outboundTCPCallout.Add();
	}

	void WFPEngine::AddOutboundIPv4PacketCallout()
	{
		m_outboundIPv4Callout = Callout(
			m_engineHandle,
			Identifiers::OutboundIPv4CalloutKey,
			Identifiers::ProviderKey,
			FWPM_LAYER_OUTBOUND_IPPACKET_V4,
			FWPM_CALLOUT_FLAG_USES_PROVIDER_CONTEXT,
			L"Outbound IPv4 Callout"
		);
		m_outboundIPv4Callout.Add();	
	}
	
	void WFPEngine::AddInboundICMPErrorCallout()
	{
		m_inboundICMPErrorCallout = Callout(
			m_engineHandle,
			Identifiers::InboundICMPErrorCalloutKey,
			Identifiers::ProviderKey,
			FWPM_LAYER_INBOUND_ICMP_ERROR_V4,
			FWPM_CALLOUT_FLAG_USES_PROVIDER_CONTEXT,
			L"Inbound ICMP Error Callout"
		);
		m_inboundICMPErrorCallout.Add();
	}

	void WFPEngine::AddOutboundICMPErrorCallout()
	{
		m_outboundICMPErrorCallout = Callout(
			m_engineHandle,
			Identifiers::OutboundICMPErrorCalloutKey,
			Identifiers::ProviderKey,
			FWPM_LAYER_OUTBOUND_ICMP_ERROR_V4,
			FWPM_CALLOUT_FLAG_USES_PROVIDER_CONTEXT,
			L"Outbound ICMP Error Callout"
		);
		m_outboundICMPErrorCallout.Add();
	}

	void WFPEngine::AddInboundIPv4PacketCallout()
	{

	}

	void WFPEngine::AddCallouts()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		AddOutboundTCPPacketCallout();
		AddOutboundIPv4PacketCallout();
		AddInboundICMPErrorCallout();
		AddOutboundICMPErrorCallout();
		AddInboundIPv4PacketCallout();
	}

	void WFPEngine::OpenFilterEngine()
	{
		FWPM_SESSION0 session = { .flags = FWPM_SESSION_FLAG_DYNAMIC };
		// Open handle to the filtering engine
		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmengineopen0
		HANDLE engineHandle = nullptr;
		const DWORD status = FwpmEngineOpen0(
			nullptr,                   // The filter engine on the local system
			RPC_C_AUTHN_DEFAULT,    // Use the Windows authentication service
			nullptr,                   // Use the calling thread&#39;s credentials
			&session,               // There are session-specific parameters
			&engineHandle     // Pointer to a variable to receive the handle
		);
		if (status != ERROR_SUCCESS)
			throw WFPError(std::source_location::current(), "FwpmProviderContextAdd3() failed", status);

		m_engineHandle = std::shared_ptr<std::remove_pointer<HANDLE>::type>(
			engineHandle,
			[](HANDLE engineHandle)
			{
				if (DWORD status = FwpmEngineClose0(engineHandle) != ERROR_SUCCESS)
					std::cout << std::format("FwpmEngineClose0() failed {:X}\n", status);
			}
		);
	}
	
	void WFPEngine::AddSublayer()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_sublayer0
		const FWPM_SUBLAYER0 sublayer = {
			.subLayerKey = Identifiers::SublayerKey,
			.displayData = {
				.name = m_sublayerName.data(),
				.description = m_sublayerDescription.data()
			},
			.flags = 0,     // FWPM_SUBLAYER_FLAG_PERSISTENT -> Causes sublayer to be persistent, surviving across BFE stop / start.
			.providerKey = const_cast<GUID*>(&Identifiers::ProviderKey),
			.weight = FWP_EMPTY
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmsublayeradd0
		const DWORD status = FwpmSubLayerAdd0(
			m_engineHandle.get(),
			&sublayer,
			nullptr
		);
		if (status != ERROR_SUCCESS)
			throw WFPError(std::source_location::current(), "FwpmSubLayerAdd0() failed", status);
		m_addedSublayer = true;
	}

	void WFPEngine::AddFilters()
	{
		if (!m_engineHandle.get())
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		AddFilter(
			m_outboundIPv4FilterName,
			FWPM_LAYER_OUTBOUND_IPPACKET_V4,
			Identifiers::SublayerKey,
			Identifiers::OutboundIPv4CalloutKey,
			FWP_EMPTY,
			FWP_ACTION_CALLOUT_INSPECTION,
			{},
			m_outboundIPv4FilterID
		);

		AddFilter(
			m_inboundICMPErrorFilterName,
			FWPM_LAYER_INBOUND_ICMP_ERROR_V4,
			Identifiers::SublayerKey,
			Identifiers::InboundICMPErrorCalloutKey,
			FWP_EMPTY,
			FWP_ACTION_CALLOUT_INSPECTION,
			{},
			m_inboundICMPErrorFilterID
		);

		AddFilter(
			m_outboundICMPErrorFilterName,
			FWPM_LAYER_OUTBOUND_ICMP_ERROR_V4,
			Identifiers::SublayerKey,
			Identifiers::OutboundICMPErrorCalloutKey,
			FWP_EMPTY,
			FWP_ACTION_CALLOUT_INSPECTION,
			{},
			m_outboundICMPErrorFilterID
		);

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_filter_condition0
		std::vector<FWPM_FILTER_CONDITION0> conditions;
		conditions.push_back({
			.fieldKey = FWPM_CONDITION_IP_PROTOCOL,
			.matchType = FWP_MATCH_EQUAL,
			.conditionValue = {
				.type = FWP_UINT8,
				.uint8 = 6// IPPROTO_TCP
			}
		});
		AddFilter(
			m_outboundTransportFilterName,
			FWPM_LAYER_OUTBOUND_TRANSPORT_V4,
			Identifiers::SublayerKey,
			Identifiers::OutboundTCPCalloutKey,
			FWP_EMPTY,
			FWP_ACTION_CALLOUT_INSPECTION,
			conditions,
			m_outboundTransportFilterID
		);
	}

	void WFPEngine::AddFilter(
		const std::wstring& filterName,
		const GUID& layerKey,
		const GUID& sublayerGuid,
		const GUID& calloutKey,
		const FWP_DATA_TYPE weightType,
		const FWP_ACTION_TYPE actionType,
		const std::vector<FWPM_FILTER_CONDITION0>& conditions,
		UINT64& filterId
	)
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");
		;
		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_filter0
		const FWPM_FILTER0 filter
		{
			.displayData {
				.name = const_cast<wchar_t*>(filterName.data())
			},
			.flags = FWPM_FILTER_FLAG_HAS_PROVIDER_CONTEXT,
			.providerKey = (GUID*)&Identifiers::ProviderKey,
			.layerKey = layerKey,
			.subLayerKey = sublayerGuid,
			.weight {
				.type = weightType
			},
			.numFilterConditions = (UINT32)conditions.size(),   // this applies to all application traffic when 0
			.filterCondition = conditions.empty() ? nullptr : const_cast<FWPM_FILTER_CONDITION0*>(&conditions[0]),
			.action = {
				.type = actionType,
				.calloutKey = calloutKey
			},
			.providerContextKey = Identifiers::ProviderContextKey
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmfilteradd0
		const DWORD status = FwpmFilterAdd0(
			m_engineHandle.get(),
			&filter,
			nullptr,           // default security desc
			&filterId
		);
		if (status != ERROR_SUCCESS)
			throw WFPError(std::source_location::current(), "FwpmFilterAdd0() failed", status);
	}
}