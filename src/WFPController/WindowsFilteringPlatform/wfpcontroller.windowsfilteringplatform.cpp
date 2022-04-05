module;

#include <stdexcept>
#include <iostream>
#include <string>
#include <format>
#include <Windows.h>
#include <fwpmu.h>
#include "../Guids.hpp"

module wfpcontroller.windowsfilteringplatform;

namespace WFPController::WindowsFilteringPlatform
{
	WindowsFilteringPlatform::~WindowsFilteringPlatform()
	{
		Close();
	}

	WindowsFilteringPlatform::WindowsFilteringPlatform()
		: m_calloutId(0), 
		m_calloutId2(0),
		m_addedProvider(false),
		m_addedSublayer(false),
		m_engineHandle(nullptr),
		m_calloutName(L"ToyDriverCallout1"),
		m_calloutName2(L"ToyDriverCallout2"),
		m_filterName(L"IPv4 outbound packet"),
		m_filterId(0),
		m_filterName2(L"Filter 2"),
		m_filterId2(0),
		m_sublayerName(L"ToyCalloutDriverSublayer"),
		m_sublayerDescription(L"Just a toy sublayer"),
		m_contextId(0)
	{
	}

	void WindowsFilteringPlatform::RegisterProvider()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		DWORD data = 321;
		std::wstring providerName = L"provider name";
		const FWPM_PROVIDER0  provider = {
			.providerKey = WFP_PROVIDER_GUID,
			.displayData = {.name = providerName.data()},
			.providerData = {
				.size = sizeof(data),
				.data = reinterpret_cast<UINT8*>(&data),
			}
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmprovideradd0
		const DWORD status = FwpmProviderAdd0(
			m_engineHandle,
			&provider,
			nullptr
		);
		if (status != ERROR_SUCCESS)
			throw std::runtime_error(std::format("Failed to add provider {}", status));
		m_addedProvider = true;
	}

	void WindowsFilteringPlatform::Close()
	{
		if (!m_engineHandle)
			return;

		if (m_filterId)
		{
			const DWORD status = FwpmFilterDeleteById0(m_engineHandle, m_filterId);
			if (status != ERROR_SUCCESS)
				std::cout << std::format("FwpmFilterDeleteById0() failed {:X}\n", status);
			m_filterId = 0;
		}

		if (m_filterId2)
		{
			const DWORD status = FwpmFilterDeleteById0(m_engineHandle, m_filterId2);
			if (status != ERROR_SUCCESS)
				std::cout << std::format("FwpmFilterDeleteById0() failed {:X}\n", status);
			m_filterId2 = 0;
		}

		DWORD status = FwpmSubLayerDeleteByKey0(m_engineHandle, &WFP_SUBLAYER_GUID);
		if (status != ERROR_SUCCESS)
			std::cout << std::format("FwpmSubLayerDeleteByKey0() failed {:X}\n", status);
		m_addedSublayer = false;
		
		status = FwpmCalloutDeleteByKey0(m_engineHandle, &WFP_OUTBOUND_IPV4_CALLOUT_GUID);
		if (status != ERROR_SUCCESS)
			std::cout << std::format("FwpmCalloutDeleteByKey0() failed {:X}\n", status);
		m_calloutId = 0;

		status = FwpmCalloutDeleteByKey0(m_engineHandle, &WFP_INBOUND_IPV4_CALLOUT_GUID);
		if (status != ERROR_SUCCESS)
			std::cout << std::format("FwpmCalloutDeleteByKey0() failed {:X}\n", status);
		m_calloutId = 0;

		//if (m_contextId)
		status = FwpmProviderContextDeleteByKey0(m_engineHandle, &WFP_PROVIDER_CONTEXT_GUID);
		//const DWORD status = FwpmProviderContextDeleteById0(m_engineHandle, m_contextId);
		if (status != ERROR_SUCCESS)
			std::cout << std::format("FwpmProviderContextDeleteById0() failed {:X}\n", status);
		m_contextId = 0;

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmproviderdeletebykey0
		status = FwpmProviderDeleteByKey0(m_engineHandle, &WFP_PROVIDER_GUID);
		if (status != ERROR_SUCCESS)
			std::cout << std::format("FwpmProviderDeleteByKey0() failed {:X}\n", status);
		m_addedProvider = false;

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmengineclose0
		status = FwpmEngineClose0(m_engineHandle);
		if (status != ERROR_SUCCESS)
			std::cout << std::format("FwpmEngineClose0() failed {:X}\n", status);
		m_engineHandle = nullptr;
	}

	void WindowsFilteringPlatform::AddContext()
	{
		std::wstring contextName = L"TODO: find a better name for this context";
		DWORD currentProcessId = GetCurrentProcessId();
		FWP_BYTE_BLOB byteBlob = { 
			.size = sizeof(currentProcessId),
			.data = reinterpret_cast<UINT8*>(&currentProcessId)
		};
		FWPM_PROVIDER_CONTEXT3 providerContext = {
			.providerContextKey = WFP_PROVIDER_CONTEXT_GUID,
			.displayData = {.name = contextName.data()},
			.providerKey = (GUID*)&WFP_PROVIDER_GUID,
			.type = FWPM_GENERAL_CONTEXT,
			.dataBuffer = &byteBlob,
		};
		//UuidCreate(&(providerContext.providerContextKey));
		const DWORD status = FwpmProviderContextAdd3(m_engineHandle, &providerContext, nullptr, &m_contextId);
		if (status != ERROR_SUCCESS)
			std::cout << std::format("FwpmProviderContextAdd3() failed {:X}\n", status);
	}

	void WindowsFilteringPlatform::AddCallouts()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		DWORD data = 123;
		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_callout0
		FWPM_CALLOUT0 mCallout = {
			.calloutKey = WFP_OUTBOUND_IPV4_CALLOUT_GUID,
			.displayData = { .name = m_calloutName.data() },
			.flags = FWPM_CALLOUT_FLAG_USES_PROVIDER_CONTEXT,
			.providerKey = (GUID*)&WFP_PROVIDER_GUID,
			.providerData = {
				.size = sizeof(data), 
				.data = reinterpret_cast<UINT8*>(&data)
			},
			.applicableLayer = FWPM_LAYER_OUTBOUND_IPPACKET_V4,
		};
		DWORD status = FwpmCalloutAdd0(
			m_engineHandle,
			&mCallout,
			nullptr,         // default security desc
			&m_calloutId
		);
		if (status != ERROR_SUCCESS)
			throw std::runtime_error(std::format("Failed to add callout 1 {}", status));

		FWPM_CALLOUT0 mCallout2 = {
			.calloutKey = WFP_INBOUND_IPV4_CALLOUT_GUID,
			.displayData = {.name = m_calloutName2.data() },
			//.flags = FWPM_CALLOUT_FLAG_USES_PROVIDER_CONTEXT,
			.providerKey = (GUID*)&WFP_PROVIDER_GUID,
			/*.providerData = {
				.size = sizeof(data),
				.data = reinterpret_cast<UINT8*>(&data)
			},*/
			.applicableLayer = FWPM_LAYER_INBOUND_IPPACKET_V4,
		};
		status = FwpmCalloutAdd0(
			m_engineHandle,
			&mCallout2,
			nullptr,         // default security desc
			&m_calloutId2
		);
		if (status != ERROR_SUCCESS)
			throw std::runtime_error(std::format("Failed to add callout 2 {}", status));
	}

	void WindowsFilteringPlatform::OpenFilterEngine()
	{
		FWPM_SESSION0 session = { 0 };
		// Open handle to the filtering engine
		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmengineopen0
		const DWORD status = FwpmEngineOpen0(
			nullptr,                   // The filter engine on the local system
			RPC_C_AUTHN_DEFAULT,    // Use the Windows authentication service
			nullptr,                   // Use the calling thread&#39;s credentials
			&session,               // There are session-specific parameters
			&m_engineHandle     // Pointer to a variable to receive the handle
		);
		if (status != ERROR_SUCCESS)
			throw std::runtime_error(std::format("Failed to open filter engine handle {}", status));
	}
	
	void WindowsFilteringPlatform::AddSublayer()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_sublayer0
		const FWPM_SUBLAYER0 sublayer = {
			.subLayerKey = WFP_SUBLAYER_GUID,
			.displayData = {
				.name = m_sublayerName.data(),
				.description = m_sublayerDescription.data()
			},
			.flags = 0,     // FWPM_SUBLAYER_FLAG_PERSISTENT -> Causes sublayer to be persistent, surviving across BFE stop / start.
			.providerKey = const_cast<GUID*>(&WFP_PROVIDER_GUID),
			.weight = FWP_EMPTY
		};
		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmsublayeradd0
		const DWORD status = FwpmSubLayerAdd0(
			m_engineHandle,
			&sublayer,
			nullptr
		);
		if (status != ERROR_SUCCESS)
			throw std::runtime_error(std::format("Failed to add sublayer {}", status));
		m_addedSublayer = true;
	}

	void WindowsFilteringPlatform::AddFilters()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		AddFilter(
			m_filterName,
			FWPM_LAYER_OUTBOUND_IPPACKET_V4,
			WFP_SUBLAYER_GUID,
			WFP_OUTBOUND_IPV4_CALLOUT_GUID,
			FWP_EMPTY,
			FWP_ACTION_CALLOUT_INSPECTION,
			m_filterId
		);

		/*AddFilter(
			m_filterName2,
			FWPM_LAYER_INBOUND_IPPACKET_V4,
			WFP_SUBLAYER_GUID,
			WFP_INBOUND_IPV4_CALLOUT_GUID,
			FWP_EMPTY,
			FWP_ACTION_CALLOUT_INSPECTION,
			m_filterId2
		);*/
	}

	void WindowsFilteringPlatform::AddFilter(
		const std::wstring& filterName,
		const GUID& layerKey,
		const GUID& sublayerGuid,
		const GUID& calloutKey,
		const FWP_DATA_TYPE weightType,
		const FWP_ACTION_TYPE actionType,
		UINT64& filterId
	)
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");
		
		DWORD data = 777;

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_filter0
		const FWPM_FILTER0 filter
		{
			.displayData {
				.name = const_cast<wchar_t*>(filterName.data())
			},
			.flags = FWPM_FILTER_FLAG_HAS_PROVIDER_CONTEXT,
			.providerKey = (GUID*)&WFP_PROVIDER_GUID,
			.providerData = {
				.size = sizeof(data),
				.data = reinterpret_cast<UINT8*>(&data)
			},
			.layerKey = layerKey,
			.subLayerKey = sublayerGuid,
			.weight {
				.type = weightType
			},
			.numFilterConditions = 0,   // this applies to all application traffic
			.action = {
				.type = actionType,
				.calloutKey = calloutKey
			},
			.providerContextKey = WFP_PROVIDER_CONTEXT_GUID
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmfilteradd0
		const DWORD status = FwpmFilterAdd0(
			m_engineHandle,
			&filter,
			nullptr,           // default security desc
			&filterId
		);
		if (status != ERROR_SUCCESS)
			throw std::runtime_error(std::format("Failed to add filters {}", status));
	}
}