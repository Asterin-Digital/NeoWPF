#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>
#include <cwctype>

#ifdef _WIN32
#include <Windows.h>
#else
#include <codecvt>
#include <locale>
#endif

namespace NeoWPF::Platform {

std::string Trim(const std::string& value)
{
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();

    if (begin >= end) {
        return {};
    }

    return std::string(begin, end);
}

std::wstring Trim(const std::wstring& value)
{
    const auto begin = std::find_if_not(value.begin(), value.end(), [](wchar_t ch) {
        return std::iswspace(static_cast<unsigned short>(ch)) != 0;
    });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](wchar_t ch) {
        return std::iswspace(static_cast<unsigned short>(ch)) != 0;
    }).base();

    if (begin >= end) {
        return {};
    }

    return std::wstring(begin, end);
}

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::wstring ToLower(std::wstring value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t character) {
        return static_cast<wchar_t>(std::towlower(character));
    });
    return value;
}

std::wstring ToUpper(std::wstring value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t character) {
        return static_cast<wchar_t>(std::towupper(character));
    });
    return value;
}

std::wstring NormalizeFileExtension(const std::wstring& extension)
{
    std::wstring normalized = Trim(extension);
    if (normalized.empty()) {
        return {};
    }

    while (!normalized.empty() && (normalized.front() == L'.' || normalized.front() == L'*')) {
        normalized.erase(normalized.begin());
    }

    if (normalized.empty() || normalized == L"*") {
        return L"*";
    }

    return L"." + normalized;
}

bool IsAsciiAlpha(wchar_t character) noexcept
{
    return (character >= L'a' && character <= L'z') ||
           (character >= L'A' && character <= L'Z');
}

bool IsAsciiDigit(wchar_t character) noexcept
{
    return character >= L'0' && character <= L'9';
}

std::vector<std::string> Split(const std::string& value, char delimiter)
{
    std::vector<std::string> parts;
    std::string current;
    for (const char ch : value) {
        if (ch == delimiter) {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
            continue;
        }
        current.push_back(ch);
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    return parts;
}

int TryParseInt(const std::string& value, int fallback)
{
    try {
        return std::stoi(value);
    } catch (...) {
        return fallback;
    }
}

bool TryParseUInt32Hex(const std::string& value, std::uint32_t& output)
{
    std::string candidate = Trim(value);
    if (candidate.empty()) {
        return false;
    }
    if (!candidate.empty() && candidate[0] == '#') {
        candidate = candidate.substr(1);
    }
    if (!candidate.empty() && candidate.rfind("0x", 0) == 0) {
        candidate = candidate.substr(2);
    }
    try {
        output = static_cast<std::uint32_t>(std::stoul(candidate, nullptr, 16));
        return true;
    } catch (...) {
        return false;
    }
}

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