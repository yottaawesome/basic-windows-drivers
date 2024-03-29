module;

#include <string>
#include <source_location>
#include <stdexcept>
#include <Windows.h>

export module wfpcontroller.winsock.winsockerror;

export namespace WFPController::WinSock
{
	class WinSockError : public std::runtime_error
	{
		public:
			virtual ~WinSockError();
			WinSockError(
				const std::source_location& location,
				const std::string& message
			);
			WinSockError(
				const std::source_location& location,
				const std::string& message,
				const DWORD errorCode
			);

		public:
			virtual DWORD GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			DWORD m_errorCode;
			std::string m_errorString;
	};
}