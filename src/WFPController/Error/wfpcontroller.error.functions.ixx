module;

#include <string>
#include <source_location>
#include <Windows.h>

export module wfpcontroller.error.functions;

export namespace WFPController::Error
{
    template<typename S>
    S FormatCode(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch) { return S(); }

    template<>
    std::string FormatCode<std::string>(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch);

    template<>
    std::wstring FormatCode<std::wstring>(const DWORD errorCode, const DWORD flags, HMODULE moduleToSearch);

    template<typename STR_T>
    STR_T TranslateErrorCode(const DWORD errorCode, const std::wstring& moduleName)
        requires std::is_same<std::string, STR_T>::value || std::is_same<std::wstring, STR_T>::value
    {
        // Retrieve the system error message for the last-error code
        HMODULE moduleHandle = moduleName.empty() ? nullptr : LoadLibraryW(moduleName.c_str());
        const DWORD flags =
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS |
            (moduleHandle ? FORMAT_MESSAGE_FROM_HMODULE : 0);
        const STR_T errorString = FormatCode<STR_T>(errorCode, flags, moduleHandle);
        if (moduleHandle)
            FreeLibrary(moduleHandle);

        return errorString;
    }

    template<typename STR_T>
    STR_T TranslateErrorCode(const DWORD errorCode)
    {
        return TranslateErrorCode<STR_T>(errorCode, L"");
    }

    std::string FormatErrorMessage(
        const std::string& errorType,
        const std::source_location& location,
        const std::string& message
    );

    std::string FormatErrorMessage(
        const std::string& errorType,
        const std::source_location& location,
        const std::string& message,
        const DWORD errorCode,
        const std::string& translatedError
    );
}