module;

#include <windows.h>
#include <winsock2.h>

export module wfpcontroller.winsock.winsockinit;

export namespace WFPController::WinSock
{
	class WinSockInit
	{
		public:
			WinSockInit();
			WinSockInit(const DWORD highVersion, const DWORD lowVersion);
			virtual ~WinSockInit();

		public:
			const WSAData& GetData() const noexcept;

		protected:
			virtual void Initialize(const DWORD highVersion, const DWORD lowVersion);

		protected:
			WSAData m_wsaData;
	};
}