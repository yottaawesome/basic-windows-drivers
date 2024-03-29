module;

#include <string>
#include <vector>

export module wfpcontroller.strings;

export namespace WFPController::Strings
{
	std::string ConvertString(const std::wstring_view wstr);
	std::wstring ConvertString(const std::string_view str);
	std::vector<std::wstring> TokeniseString(
		const std::wstring& stringToTokenise,
		const std::wstring& delimiter
	);
	std::wstring Replace(
		std::wstring source,
		const std::wstring& from,
		const std::wstring& to
	);
	std::wstring Erase(std::wstring source, const wchar_t what);
	bool DoCaseInsensitiveMatch(std::string str1, std::string str2);
	bool DoCaseInsensitiveMatch(std::wstring str1, std::wstring str2);
}