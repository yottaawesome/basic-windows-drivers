module;

#include <string>
#include <Windows.h>
#include <fwpmu.h>

export module wfpcontroller.windowsfilteringplatform;

export namespace WFPController::WindowsFilteringPlatform
{
	export class WindowsFilteringPlatform
	{
		public:
			virtual ~WindowsFilteringPlatform();
			WindowsFilteringPlatform();

		public:
			virtual void Close();
			virtual void OpenFilterEngine();
			virtual void RegisterProvider();
			virtual void AddContext();
			virtual void AddCallouts();
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
			HANDLE m_engineHandle; // handle for the open session to the filter engine
			bool m_addedProvider;
			bool m_addedSublayer;
			std::wstring m_sublayerName;
			std::wstring m_sublayerDescription;

			UINT64 m_filterId;
			std::wstring m_filterName;

			UINT64 m_filterId2;
			std::wstring m_filterName2;

			std::wstring m_calloutName;
			unsigned m_calloutId;

			std::wstring m_calloutName2;
			unsigned m_calloutId2;

			UINT64 m_contextId;
	};
}