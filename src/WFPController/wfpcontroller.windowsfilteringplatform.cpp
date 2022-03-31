module;

#include <stdexcept>
#include <iostream>
#include <string>
#include <format>
#include <Windows.h>
#include <fwpmu.h>
#include "Guids.hpp"

module wfpcontroller.windowsfilteringplatform;

namespace WFPController
{
	WindowsFilteringPlatform::~WindowsFilteringPlatform()
	{
		Close();
	}

	WindowsFilteringPlatform::WindowsFilteringPlatform()
		: m_calloutId(0), 
		m_addedProvider(false),
		m_addedSublayer(false),
		m_engineHandle(nullptr),
		m_calloutName(L"ToyDriverCallout"),
		m_filterName(L"IPv4 outbound packet"),
		m_filterId(0),
		m_sublayerName(L"ToyCalloutDriverSublayer"),
		m_sublayerDescription(L"Just a toy sublayer")
	{
	}

	void WindowsFilteringPlatform::RegisterProvider()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		std::wstring providerName = L"provider name";
		const FWPM_PROVIDER0  provider = {
			.providerKey = WFP_PROVIDER_GUID,
			.displayData = {.name = providerName.data()}
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

		if (m_addedSublayer)
		{
			const DWORD status = FwpmSubLayerDeleteByKey0(m_engineHandle, &WFP_SUBLAYER_GUID);
			if (status != ERROR_SUCCESS)
				std::cout << std::format("FwpmSubLayerDeleteByKey0() failed {:X}\n", status);
			m_addedSublayer = false;
		}
		
		if (m_calloutId)
		{
			const DWORD status = FwpmCalloutDeleteByKey0(m_engineHandle, &WFP_TEST_CALLOUT);
			if (status != ERROR_SUCCESS)
				std::cout << std::format("FwpmCalloutDeleteByKey0() failed {:X}\n", status);
			m_calloutId = 0;
		}

		if (m_addedProvider)
		{
			// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmproviderdeletebykey0
			const DWORD status = FwpmProviderDeleteByKey0(m_engineHandle, &WFP_PROVIDER_GUID);
			if (status != ERROR_SUCCESS)
				std::cout << std::format("FwpmProviderDeleteByKey0() failed {:X}\n", status);
			m_addedProvider = false;
		}

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmengineclose0
		const DWORD status = FwpmEngineClose0(m_engineHandle);
		if (status != ERROR_SUCCESS)
			std::cout << std::format("FwpmEngineClose0() failed {:X}\n", status);
		m_engineHandle = nullptr;
	}

	void WindowsFilteringPlatform::AddCallouts()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		FWPM_CALLOUT0 mCallout = {
			.calloutKey = WFP_TEST_CALLOUT,
			.displayData = { .name = m_calloutName.data() },
			.providerKey = (GUID*)&WFP_PROVIDER_GUID,
			.applicableLayer = FWPM_LAYER_OUTBOUND_IPPACKET_V4,
		};
		const DWORD status = FwpmCalloutAdd0(
			m_engineHandle,
			&mCallout,
			nullptr,         // default security desc
			&m_calloutId
		);
		if (status != ERROR_SUCCESS)
			throw std::runtime_error(std::format("Failed to add callout {}", status));
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
			WFP_TEST_CALLOUT,
			FWP_EMPTY,
			FWP_ACTION_CALLOUT_INSPECTION
		);
	}

	void WindowsFilteringPlatform::AddFilter(
		const std::wstring& filterName,
		const GUID& layerKey,
		const GUID& sublayerGuid,
		const GUID& calloutKey,
		const FWP_DATA_TYPE weightType,
		const FWP_ACTION_TYPE actionType
	)
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_filter0
		const FWPM_FILTER0 filter
		{
			.displayData {
				.name = const_cast<wchar_t*>(filterName.data())
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
		};

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmfilteradd0
		const DWORD status = FwpmFilterAdd0(
			m_engineHandle,
			&filter,
			nullptr,           // default security desc
			&m_filterId
		);
		if (status != ERROR_SUCCESS)
			throw std::runtime_error(std::format("Failed to add filters {}", status));
	}
}