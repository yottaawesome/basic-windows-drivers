module;

#include <memory>
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

export module wfpcontroller.winsock.uniqueptrs;

export namespace WFPController::WinSock
{
	struct AddrInfoWDeleter final
	{
		void operator()(ADDRINFOW* obj)
		{
			FreeAddrInfoW(obj);
		}
	};
	using AddrInfoWUniquePtr = std::unique_ptr<ADDRINFOW, AddrInfoWDeleter>;
}
