module;

#include <string>
#include <stdexcept>
#include <source_location>
#include <Windows.h>

export module wfpcontroller.error.win32error;

export namespace WFPController::Error
{
	class Win32Error : public std::runtime_error
	{
		public:
			virtual ~Win32Error();
			Win32Error(const Win32Error& other);
			Win32Error(Win32Error&& other) noexcept;
			Win32Error(
				const std::source_location& location, 
				const std::string& msg
			);
			Win32Error(
				const std::source_location& location, 
				const std::string& msg, 
				const DWORD errorCode
			);
			Win32Error(
				const std::source_location& location, 
				const std::string& msg, 
				const DWORD errorCode, 
				const std::wstring& moduleName
			);

		public:
			virtual Win32Error& operator=(const Win32Error& other);
			virtual Win32Error& operator=(Win32Error&& other) noexcept;

		public:
			virtual DWORD GetErrorCode() const noexcept;
			virtual const char* what() const noexcept override;

		protected:
			DWORD m_errorCode;
			std::string m_errorString;
	};
}