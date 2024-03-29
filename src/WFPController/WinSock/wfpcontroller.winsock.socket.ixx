module;

#include <string>
#include <vector>
#include <windows.h>
#include <winsock2.h>

export module wfpcontroller.winsock.socket;
import wfpcontroller.winsock.uniqueptrs;

export namespace WFPController::WinSock
{
	// Based on https://docs.microsoft.com/en-us/windows/win32/winsock/winsock-client-application
	class Socket
	{
		public:
			static const SOCKET InvalidSocket;

		public:
			virtual ~Socket();
			Socket();
			Socket(const Socket& other) = delete;
			Socket(Socket&& other) noexcept;
			Socket(const std::wstring host, const unsigned portNumber);

		public:
			virtual Socket& operator=(const Socket& other) = delete;
			virtual Socket& operator=(Socket&& other) noexcept;

		public:
			virtual void Connect();
			virtual void Close();
			virtual void Send(const std::vector<std::byte>& data);
			virtual std::vector<std::byte> Receive(const unsigned bytesToRead);
			virtual void SetSocketTTL(const DWORD ttl);
			virtual void SetPreconnectTTL(const DWORD ttl);

		public:
			virtual const std::wstring& GetHost() const noexcept;
			virtual unsigned GetPort() const noexcept;
			virtual SOCKET GetHandle() const noexcept;

		protected:
			virtual Socket& Move(Socket& other) noexcept;

		protected:
			std::wstring m_host;
			unsigned m_portNumber;
			SOCKET m_socket; // doesn't work with unique_ptr
			int m_addressFamily;
			DWORD m_preconnectTTL;
	};
}