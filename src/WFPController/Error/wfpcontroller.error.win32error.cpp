module;

#include <stdexcept>
#include <source_location>
#include <string>
#include <format>
// See #include <winnt.h> for FAST_FAIL error codes
#include "Windows.h"

module wfpcontroller.error.win32error;
import wfpcontroller.error.functions;

namespace WFPController::Error
{
	Win32Error::~Win32Error() {}

	Win32Error::Win32Error(const Win32Error& other) = default;
	Win32Error::Win32Error(Win32Error&& other) noexcept = default;
	Win32Error& Win32Error::operator=(const Win32Error& other) = default;
	Win32Error& Win32Error::operator=(Win32Error&& other) noexcept = default;

	Win32Error::Win32Error(
		const std::source_location& location,
		const std::string& msg
	)
		: std::runtime_error(msg),
		m_errorCode(0)
	{
		m_errorString = Error::FormatErrorMessage("Win32", location, msg);
	}

	Win32Error::Win32Error(
		const std::source_location& location,
		const std::string& msg,
		const DWORD errorCode
	)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Error::TranslateErrorCode<std::string>(errorCode);
		m_errorString = Error::FormatErrorMessage("Win32", location, msg, errorCode, m_errorString);
	}

	Win32Error::Win32Error(
		const std::source_location& location,
		const std::string& msg,
		const DWORD errorCode,
		const std::wstring& moduleName
	)
		: std::runtime_error(msg),
		m_errorCode(errorCode)
	{
		m_errorString = Error::TranslateErrorCode<std::string>(errorCode, moduleName);
		m_errorString = Error::FormatErrorMessage("Win32", location, msg, errorCode, m_errorString);
	}

	DWORD Win32Error::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}

	const char* Win32Error::what() const noexcept
	{
		return m_errorString.c_str();
	}
}
