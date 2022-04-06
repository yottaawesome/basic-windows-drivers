module;

#include <string>
#include <source_location>
#include <stdexcept>
#include <Windows.h>

module wfpcontroller.wfp.wfperror;
import wfpcontroller.error.functions;

namespace WFPController::WFP
{
	WFPError::~WFPError() { }

	WFPError::WFPError(
		const std::source_location& location,
		const std::string& message
	) : std::runtime_error(""),
		m_errorCode(0)
	{
		m_errorString = Error::FormatErrorMessage("WFP", location, message);
	}

	WFPError::WFPError(
		const std::source_location& location,
		const std::string& message,
		const DWORD errorCode
	)
		: std::runtime_error(""),
		m_errorCode(errorCode)
	{
		m_errorString = Error::TranslateErrorCode<std::string>(errorCode, L"Fwpuclnt.dll");
		m_errorString = Error::FormatErrorMessage("WFP", location, message, errorCode, m_errorString);
	}

	DWORD WFPError::GetErrorCode() const noexcept
	{
		return m_errorCode;
	}

	const char* WFPError::what() const noexcept
	{
		return m_errorString.c_str();
	}
}