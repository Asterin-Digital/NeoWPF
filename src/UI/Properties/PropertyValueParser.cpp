#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/Thickness.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::wstring ToWideString(std::string_view value)
{
    std::wstring result;
    result.reserve(value.size());
    for (const char ch : value) {
        result.push_back(static_cast<wchar_t>(ch));
    }
    return result;
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
        return std::string();
    }
    return std::string(begin, end);
}

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool TryParseColor(const std::string& rawValue, NeoWPF::UI::Color& color)
{
    std::string value = Trim(rawValue);
    if (value.empty()) {
        return false;
    }

    if (value.size() > 1 && value[0] == '#') {
        value = value.substr(1);
    }
    if (value.size() > 2 && value.rfind("0x", 0) == 0) {
        value = value.substr(2);
    }
    if (value.size() == 6 || value.size() == 8) {
        std::uint32_t parsed = 0;
        try {
            parsed = static_cast<std::uint32_t>(std::stoul(value, nullptr, 16));
        } catch (...) {
            return false;
        }

        if (value.size() == 6) {
            color = NeoWPF::UI::Color::FromArgb(0xFF000000u | parsed);
            return true;
        }

        color = NeoWPF::UI::Color::FromArgb(parsed);
        return true;
    }

    return false;
}

bool TryParseThickness(const std::string& rawValue, NeoWPF::UI::Thickness& thickness)
{
    std::string value = Trim(rawValue);
    if (value.empty()) {
        return false;
    }

    std::vector<std::string> parts;
    std::string current;
    for (char ch : value) {
        if (ch == ',') {
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

    if (parts.empty()) {
        return false;
    }

    std::vector<int> numbers;
    numbers.reserve(parts.size());
    for (const auto& part : parts) {
        const auto trimmed = Trim(part);
        if (trimmed.empty()) {
            return false;
        }
        try {
            numbers.push_back(std::stoi(trimmed));
        } catch (...) {
            return false;
        }
    }

    if (numbers.size() == 1) {
        thickness = NeoWPF::UI::Thickness(numbers[0]);
        return true;
    }
    if (numbers.size() == 2) {
        thickness = NeoWPF::UI::Thickness(numbers[0], numbers[1], numbers[0], numbers[1]);
        return true;
    }
    if (numbers.size() == 4) {
        thickness = NeoWPF::UI::Thickness(numbers[0], numbers[1], numbers[2], numbers[3]);
        return true;
    }

    return false;
}

bool TryParseHorizontalAlignment(const std::string& rawValue, NeoWPF::UI::HorizontalAlignment& alignment)
{
    const std::string value = ToLower(Trim(rawValue));
    if (value == "left") { alignment = NeoWPF::UI::HorizontalAlignment::Left; return true; }
    if (value == "center") { alignment = NeoWPF::UI::HorizontalAlignment::Center; return true; }
    if (value == "right") { alignment = NeoWPF::UI::HorizontalAlignment::Right; return true; }
    if (value == "stretch") { alignment = NeoWPF::UI::HorizontalAlignment::Stretch; return true; }
    return false;
}

bool TryParseVerticalAlignment(const std::string& rawValue, NeoWPF::UI::VerticalAlignment& alignment)
{
    const std::string value = ToLower(Trim(rawValue));
    if (value == "top") { alignment = NeoWPF::UI::VerticalAlignment::Top; return true; }
    if (value == "center") { alignment = NeoWPF::UI::VerticalAlignment::Center; return true; }
    if (value == "bottom") { alignment = NeoWPF::UI::VerticalAlignment::Bottom; return true; }
    if (value == "stretch") { alignment = NeoWPF::UI::VerticalAlignment::Stretch; return true; }
    return false;
}

}

namespace NeoWPF::UI::Properties {

bool DependencyProperty::TryParseValue(const std::string& rawValue, std::any& parsedValue) const
{
    if (m_metadata.ValueParser) {
        return m_metadata.ValueParser(rawValue, parsedValue);
    }

    if (m_propertyType == typeid(std::wstring)) {
        parsedValue = ToWideString(rawValue);
        return true;
    }

    if (m_propertyType == typeid(std::string)) {
        parsedValue = rawValue;
        return true;
    }

    if (m_propertyType == typeid(int)) {
        parsedValue = std::stoi(rawValue);
        return true;
    }

    if (m_propertyType == typeid(bool)) {
        const std::string value = ToLower(Trim(rawValue));
        parsedValue = value == "true" || value == "1" || value == "yes";
        return true;
    }

    if (m_propertyType == typeid(std::uint32_t)) {
        std::string value = Trim(rawValue);
        if (!value.empty() && value[0] == '#') {
            value = value.substr(1);
        }
        if (!value.empty() && value.rfind("0x", 0) == 0) {
            value = value.substr(2);
        }
        parsedValue = static_cast<std::uint32_t>(std::stoul(value, nullptr, 16));
        return true;
    }

    if (m_propertyType == typeid(NeoWPF::UI::Color)) {
        NeoWPF::UI::Color color{};
        if (TryParseColor(rawValue, color)) {
            parsedValue = color;
            return true;
        }
        return false;
    }

    if (m_propertyType == typeid(NeoWPF::UI::Thickness)) {
        NeoWPF::UI::Thickness thickness{};
        if (TryParseThickness(rawValue, thickness)) {
            parsedValue = thickness;
            return true;
        }
        return false;
    }

    if (m_propertyType == typeid(NeoWPF::UI::HorizontalAlignment)) {
        NeoWPF::UI::HorizontalAlignment alignment{};
        if (TryParseHorizontalAlignment(rawValue, alignment)) {
            parsedValue = alignment;
            return true;
        }
        return false;
    }

    if (m_propertyType == typeid(NeoWPF::UI::VerticalAlignment)) {
        NeoWPF::UI::VerticalAlignment alignment{};
        if (TryParseVerticalAlignment(rawValue, alignment)) {
            parsedValue = alignment;
            return true;
        }
        return false;
    }

    return false;
}

}
