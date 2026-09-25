#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>
#include <cwctype>
#include <iterator>
#include <limits>
#include <type_traits>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace NeoWPF::Platform {

namespace {

#ifndef _WIN32


static_assert(sizeof(wchar_t) == 4,
    "WideToUtf8/Utf8ToWide assume a 32-bit (UTF-32) wchar_t outside Windows.");

constexpr char32_t kMaxCodePoint = 0x10FFFF;
constexpr char32_t kSurrogateMin = 0xD800;
constexpr char32_t kSurrogateMax = 0xDFFF;

bool IsValidCodePoint(char32_t codePoint) noexcept
{
    return codePoint <= kMaxCodePoint &&
           !(codePoint >= kSurrogateMin && codePoint <= kSurrogateMax);
}


bool AppendUtf8(char32_t codePoint, std::string& output)
{
    if (!IsValidCodePoint(codePoint)) {
        return false;
    }

    if (codePoint <= 0x7F) {
        output.push_back(static_cast<char>(codePoint));
    } else if (codePoint <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
        output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else if (codePoint <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
        output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else {
        output.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
        output.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    }
    return true;
}

bool DecodeUtf8CodePoint(std::string::const_iterator& it, std::string::const_iterator end, char32_t& codePoint)
{
    const auto firstByte = static_cast<unsigned char>(*it);

    std::size_t sequenceLength = 0;
    char32_t value = 0;
    char32_t minimumValue = 0;

    if ((firstByte & 0x80) == 0x00) {
        sequenceLength = 1;
        value = firstByte;
        minimumValue = 0x0;
    } else if ((firstByte & 0xE0) == 0xC0) {
        sequenceLength = 2;
        value = firstByte & 0x1F;
        minimumValue = 0x80;
    } else if ((firstByte & 0xF0) == 0xE0) {
        sequenceLength = 3;
        value = firstByte & 0x0F;
        minimumValue = 0x800;
    } else if ((firstByte & 0xF8) == 0xF0) {
        sequenceLength = 4;
        value = firstByte & 0x07;
        minimumValue = 0x10000;
    } else {
        return false;
    }

    if (static_cast<std::size_t>(std::distance(it, end)) < sequenceLength) {
        return false;
    }

    auto cursor = it;
    ++cursor;
    for (std::size_t i = 1; i < sequenceLength; ++i, ++cursor) {
        const auto continuationByte = static_cast<unsigned char>(*cursor);
        if ((continuationByte & 0xC0) != 0x80) {
            return false;
        }
        value = (value << 6) | (continuationByte & 0x3F);
    }

    if (value < minimumValue || !IsValidCodePoint(value)) {
        return false;
    }

    codePoint = value;
    it = cursor;
    return true;
}

#endif // !_WIN32

} 

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

std::string WideToUtf8(const std::wstring& input)
{
    if (input.empty()) {
        return {};
    }

#ifdef _WIN32
    if (input.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)())) {
        return {};
    }

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
    std::string output;
    output.reserve(input.size());

    for (const wchar_t wideChar : input) {
        const auto codePoint = static_cast<char32_t>(static_cast<std::make_unsigned_t<wchar_t>>(wideChar));
        if (!AppendUtf8(codePoint, output)) {
            return {};
        }
    }
    return output;
#endif
}

std::wstring Utf8ToWide(const std::string& input)
{
    if (input.empty()) {
        return {};
    }

#ifdef _WIN32
    if (input.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)())) {
        return {};
    }

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
    std::wstring output;
    output.reserve(input.size());

    auto it = input.begin();
    const auto end = input.end();
    while (it != end) {
        char32_t codePoint = 0;
        if (!DecodeUtf8CodePoint(it, end, codePoint)) {
            return {};
        }
        output.push_back(static_cast<wchar_t>(codePoint));
    }
    return output;
#endif
}

}