#include <NeoWPF/Platform/PlatformUtils.h>
#include <Cudev/Cudev.h>

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <limits>
#include <utility>

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
        return std::iswspace(ch) != 0;
    });
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](wchar_t ch) {
        return std::iswspace(ch) != 0;
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

    if (!candidate.empty() && (candidate[0] == '+' || candidate[0] == '-')) {
        return false;
    }
    try {
        const unsigned long parsed = std::stoul(candidate, nullptr, 16);
        if (parsed > (std::numeric_limits<std::uint32_t>::max)()) {
            return false;
        }
        output = static_cast<std::uint32_t>(parsed);
        return true;
    } catch (...) {
        return false;
    }
}

bool IsValidUuid(const std::string& value) noexcept
{
    if (value.size() != 36) {
        return false;
    }

    for (std::size_t index = 0; index < value.size(); ++index) {
        if (index == 8 || index == 13 || index == 18 || index == 23) {
            if (value[index] != '-') {
                return false;
            }
            continue;
        }

        const char character = value[index];
        const bool isHexDigit =
            (character >= '0' && character <= '9') ||
            (character >= 'a' && character <= 'f') ||
            (character >= 'A' && character <= 'F');
        if (!isHexDigit) {
            return false;
        }
    }

    return true;
}

std::string WideToUtf8(const std::wstring& input)
{
    auto converted = Cudev::Wide::ToUtf8(input);
    return converted.succeeded() ? std::move(converted).value() : std::string{};
}

std::wstring Utf8ToWide(const std::string& input)
{
    auto converted = Cudev::Wide::FromUtf8(input);
    return converted.succeeded() ? std::move(converted).value() : std::wstring{};
}

}