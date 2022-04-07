module;

#include <Windows.h>
#include <fwpmu.h>

module wfpcontroller.wfp.filter;

namespace WFPController::WFP
{
	void Filter::Remove()
	{
		//	if (m_filterId)
//	{
//		const DWORD status = FwpmFilterDeleteById0(m_engineHandle, m_filterId);
//		if (status != ERROR_SUCCESS)
//			std::cout << std::format("FwpmFilterDeleteById0() failed {:X}\n", status);
//		m_filterId = 0;
//	}
	}
}

//void WindowsFilteringPlatform::Close()
//{
//	if (!m_engineHandle)
//		return;
//
//	if (m_filterId)
//	{
//		const DWORD status = FwpmFilterDeleteById0(m_engineHandle, m_filterId);
//		if (status != ERROR_SUCCESS)
//			std::cout << std::format("FwpmFilterDeleteById0() failed {:X}\n", status);
//		m_filterId = 0;
//	}
//
//	if (m_filterId2)
//	{
//		const DWORD status = FwpmFilterDeleteById0(m_engineHandle, m_filterId2);
//		if (status != ERROR_SUCCESS)
//			std::cout << std::format("FwpmFilterDeleteById0() failed {:X}\n", status);
//		m_filterId2 = 0;
//	}
//
//	DWORD status = FwpmSubLayerDeleteByKey0(m_engineHandle, &WFP_SUBLAYER_GUID);
//	if (status != ERROR_SUCCESS)
//		std::cout << std::format("FwpmSubLayerDeleteByKey0() failed {:X}\n", status);
//	m_addedSublayer = false;
//
//	status = FwpmCalloutDeleteByKey0(m_engineHandle, &WFP_OUTBOUND_IPV4_CALLOUT_GUID);
//	if (status != ERROR_SUCCESS)
//		std::cout << std::format("FwpmCalloutDeleteByKey0() failed {:X}\n", status);
//	m_calloutId = 0;
//
//	status = FwpmCalloutDeleteByKey0(m_engineHandle, &WFP_INBOUND_IPV4_CALLOUT_GUID);
//	if (status != ERROR_SUCCESS)
//		std::cout << std::format("FwpmCalloutDeleteByKey0() failed {:X}\n", status);
//	m_calloutId = 0;
//
//	//if (m_contextId)
//	status = FwpmProviderContextDeleteByKey0(m_engineHandle, &WFP_PROVIDER_CONTEXT_GUID);
//	//const DWORD status = FwpmProviderContextDeleteById0(m_engineHandle, m_contextId);
//	if (status != ERROR_SUCCESS)
//		std::cout << std::format("FwpmProviderContextDeleteById0() failed {:X}\n", status);
//	m_contextId = 0;
//
//	// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmproviderdeletebykey0
//	status = FwpmProviderDeleteByKey0(m_engineHandle, &WFP_PROVIDER_GUID);
//	if (status != ERROR_SUCCESS)
//		std::cout << std::format("FwpmProviderDeleteByKey0() failed {:X}\n", status);
//	m_addedProvider = false;
//
//	// https://docs.microsoft.com/en-us/windows/win32/api/fwpmu/nf-fwpmu-fwpmengineclose0
//	status = FwpmEngineClose0(m_engineHandle);
//	if (status != ERROR_SUCCESS)
//		std::cout << std::format("FwpmEngineClose0() failed {:X}\n", status);
//	m_engineHandle = nullptr;
//}