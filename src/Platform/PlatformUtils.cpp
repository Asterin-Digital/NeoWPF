#include <NeoWPF/Platform/PlatformUtils.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <codecvt>
#include <locale>
#endif

namespace NeoWPF::Platform {

std::string WideToUtf8(const std::wstring& input)
{
    if (input.empty()) {
        return {};
    }

#ifdef _WIN32
    const int requiredSize = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        input.data(),
        static_cast<int>(input.size()),
        nullptr,
        0,
        nullptr,
        nullptr);
    if (requiredSize <= 0) {
        return {};
    }

    std::string output(static_cast<size_t>(requiredSize), '\0');
    if (WideCharToMultiByte(
            CP_UTF8,
            WC_ERR_INVALID_CHARS,
            input.data(),
            static_cast<int>(input.size()),
            output.data(),
            requiredSize,
            nullptr,
            nullptr) <= 0) {
        return {};
    }
    return output;
#else
    try {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes(input);
    } catch (const std::range_error&) {
        return {};
    }
#endif
}

std::wstring Utf8ToWide(const std::string& input)
{
    if (input.empty()) {
        return {};
    }

#ifdef _WIN32
    const int requiredSize = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        input.data(),
        static_cast<int>(input.size()),
        nullptr,
        0);
    if (requiredSize <= 0) {
        return {};
    }

    std::wstring output(static_cast<size_t>(requiredSize), L'\0');
    if (MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            input.data(),
            static_cast<int>(input.size()),
            output.data(),
            requiredSize) <= 0) {
        return {};
    }
    return output;
#else
    try {
        return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.from_bytes(input);
    } catch (const std::range_error&) {
        return {};
    }
#endif
}

}