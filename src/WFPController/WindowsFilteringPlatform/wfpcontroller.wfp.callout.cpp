module;

#include <string>
#include <memory>
#include <source_location>
#include <iostream>
#include <format>
#include <stdexcept>
#include <Windows.h>
#include <fwpmu.h>

module wfpcontroller.wfp.callout;
import wfpcontroller.wfp.wfperror;
namespace WFPController::WFP
{
	Callout::Callout()
		: m_calloutKey{ 0 },
		m_providerKey{ 0 },
		m_applicableLayerKey{ 0 },
		m_calloutId(0),
		m_flags(0)
	{ }

	Callout::~Callout()
	{
		Remove();
	}
	
	Callout::Callout(Callout&& other) noexcept 
	{
		Move(other);
	};

	Callout::Callout(
		std::shared_ptr<std::remove_pointer<HANDLE>::type>& engineHandle,
		const GUID& calloutKey,
		const GUID& providerKey,
		const GUID& applicableLayerKey,
		const unsigned flags,
		std::wstring displayName
	)
		: m_engineHandle(engineHandle),
		m_calloutKey(calloutKey),
		m_providerKey(providerKey),
		m_applicableLayerKey(applicableLayerKey),
		m_displayName(std::move(displayName)),
		m_calloutId(0),
		m_flags(flags)
	{
		if (!m_engineHandle)
			throw std::invalid_argument("Engine handle cannot be null");
	}
	
	Callout& Callout::operator=(Callout&& other) noexcept 
	{
		return Move(other);
	}

	Callout& Callout::Move(Callout& other) noexcept
	{
		Remove();

		m_displayName = std::move(other.m_displayName);
		m_calloutKey = other.m_calloutKey;
		m_providerKey = other.m_providerKey;
		m_applicableLayerKey = other.m_applicableLayerKey;
		m_engineHandle = std::move(other.m_engineHandle);
		m_calloutId = other.m_calloutId;
		m_flags = other.m_flags;
		
		other.m_calloutId = 0;

		return *this;
	}

	void Callout::Add()
	{
		if (!m_engineHandle)
			throw std::invalid_argument(__FUNCSIG__": engineHandle");

		// https://docs.microsoft.com/en-us/windows/win32/api/fwpmtypes/ns-fwpmtypes-fwpm_callout0
		FWPM_CALLOUT0 callout = {
			.calloutKey = m_calloutKey,
			.displayData = {.name = m_displayName.data() },
			.flags = m_flags,
			.providerKey = &m_providerKey,
			.applicableLayer = m_applicableLayerKey,
		};
		const DWORD status = FwpmCalloutAdd0(
			m_engineHandle.get(),
			&callout,
			nullptr,         // default security desc
			&m_calloutId
		);
		if (status != ERROR_SUCCESS) throw WFPError(
			std::source_location::current(),
			"Failed to add callout",
			status
		);
	}

	void Callout::Remove()
	{
		if (!m_calloutId || !m_engineHandle)
			return;

		const DWORD status = FwpmCalloutDeleteByKey0(m_engineHandle.get(), &m_calloutKey);
		m_calloutId = 0;
		if (status != ERROR_SUCCESS)
			std::cerr << std::format("FwpmCalloutDeleteByKey0() failed {:X}\n", status);
		m_engineHandle = nullptr;
	}
}
