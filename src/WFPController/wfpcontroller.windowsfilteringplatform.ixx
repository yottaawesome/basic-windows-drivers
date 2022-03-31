module;

#include <string>
#include <Windows.h>
#include <fwpmu.h>

export module wfpcontroller.windowsfilteringplatform;

export namespace WFPController
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
				const FWP_ACTION_TYPE actionType
			);

		protected:
			HANDLE m_engineHandle; // handle for the open session to the filter engine
			unsigned m_calloutId;
			bool m_addedProvider;
			bool m_addedSublayer;
			UINT64 m_filterId;
			std::wstring m_calloutName;
			std::wstring m_filterName;
			std::wstring m_sublayerName;
			std::wstring m_sublayerDescription;
	};
}