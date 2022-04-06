module;

#include <stdexcept>
#include <string>
#include <source_location>
#include <Windows.h>

export module wfpcontroller.wfp.wfperror;

export namespace WFPController::WFP
{
	class WFPError : public std::runtime_error
	{
		public:
			virtual ~WFPError();
			WFPError(
				const std::source_location& location,
				const std::string& message
			);
			WFPError(
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