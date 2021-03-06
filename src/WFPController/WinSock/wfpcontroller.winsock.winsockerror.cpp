module;

#include <string>
#include <source_location>
#include <stdexcept>
#include <Windows.h>

module wfpcontroller.winsock.winsockerror;
import wfpcontroller.error.functions;

namespace WFPController::WinSock
{
	WinSockError::~WinSockError() { }

	WinSockError::WinSockError(
		const std::source_location& location,
		const std::string& message
	) : std::runtime_error(""),
		m_errorCode(0)
	{
		m_errorString = Error::FormatErrorMessage("WinSock", location, message);
	}

	WinSockError::WinSockError(
		const std::source_location& location,
		const std::string& message,
		const DWORD errorCode
	)
		: std::runtime_error(""),
		m_errorCode(errorCode)
	{
		m_errorString = Error::TranslateErrorCode<std::string>(errorCode, L"Ws2_32.dll");
		m_errorString = Error::FormatErrorMessage("WinSock", location, message, errorCode, m_errorString);
	}

	DWORD WinSockError::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}

	const char* WinSockError::what() const noexcept
	{
		return m_errorString.c_str();
	}
}