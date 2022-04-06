module;

#include <string>
#include <memory>
#include <Windows.h>

export module wfpcontroller.wfp.callout;

export namespace WFPController::WFP
{
	class Callout
	{
		public:
			virtual ~Callout();
			Callout();
			Callout(const Callout&) = delete;
			Callout(Callout&& other) noexcept;
			Callout(
				std::shared_ptr<std::remove_pointer<HANDLE>::type>& engineHandle,
				const GUID& calloutKey,
				const GUID& providerKey,
				const GUID& applicableLayerKey,
				const unsigned flags,
				std::wstring displayName
			);

		public:
			virtual Callout& operator=(const Callout&) = delete;
			virtual Callout& operator=(Callout&& other) noexcept;

		public:
			virtual void Add();
			virtual void Remove();

		protected:
			virtual Callout& Move(Callout& other) noexcept;

		protected:
			std::wstring m_displayName;
			GUID m_calloutKey;
			GUID m_providerKey;
			GUID m_applicableLayerKey;
			std::shared_ptr<std::remove_pointer<HANDLE>::type> m_engineHandle;
			unsigned m_calloutId;
			unsigned m_flags;
	};
}