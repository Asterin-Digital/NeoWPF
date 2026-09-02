#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/Thickness.h>
#include <NeoWPF/UI/Core/Media/Spacing.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>

#include <cstdint>
#include <string>
#include <vector>

namespace NeoWPF::UI::Properties {

bool StringConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    convertedValue = rawValue;
    return true;
}

bool WideStringConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    convertedValue = std::wstring(rawValue.begin(), rawValue.end());
    return true;
}

bool IntConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    try {
        std::size_t parsed = 0;
        const int value = std::stoi(NeoWPF::Platform::Trim(rawValue), &parsed);
        const auto input = NeoWPF::Platform::Trim(rawValue);
        if (parsed != input.size()) {
            return false;
        }
        convertedValue = value;
        return true;
    } catch (...) {
        return false;
    }
}

bool FloatConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    try {
        std::size_t parsed = 0;
        const auto input = NeoWPF::Platform::Trim(rawValue);
        const float value = std::stof(input, &parsed);
        if (parsed != input.size()) {
            return false;
        }
        convertedValue = value;
        return true;
    } catch (...) {
        return false;
    }
}

bool BoolConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value == "true") {
        convertedValue = true;
        return true;
    }
    if (value == "false") {
        convertedValue = false;
        return true;
    }
    return false;
}

bool ColorConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    auto value = NeoWPF::Platform::Trim(rawValue);
    if (value.empty() || value.front() != '#') {
        return false;
    }
    value.erase(0, 1);
    if (value.size() != 6 && value.size() != 8) {
        return false;
    }
    try {
        auto argb = static_cast<std::uint32_t>(std::stoul(value, nullptr, 16));
        if (value.size() == 6) {
            argb |= 0xFF000000u;
        }
        convertedValue = NeoWPF::UI::Color::FromArgb(argb);
        return true;
    } catch (...) {
        return false;
    }
}

bool ThicknessConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    const auto parts = NeoWPF::Platform::Split(NeoWPF::Platform::Trim(rawValue), ',');
    if (parts.size() != 1 && parts.size() != 2 && parts.size() != 4) {
        return false;
    }
    try {
        std::vector<int> values;
        for (const auto& part : parts) {
            std::size_t parsed = 0;
            const auto input = NeoWPF::Platform::Trim(part);
            values.push_back(std::stoi(input, &parsed));
            if (parsed != input.size()) {
                return false;
            }
        }
        if (values.size() == 1) {
            convertedValue = NeoWPF::UI::Thickness(values[0]);
        } else if (values.size() == 2) {
            convertedValue = NeoWPF::UI::Thickness(values[0], values[1], values[0], values[1]);
        } else {
            convertedValue = NeoWPF::UI::Thickness(values[0], values[1], values[2], values[3]);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool SpacingConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    const auto parts = NeoWPF::Platform::Split(NeoWPF::Platform::Trim(rawValue), ',');
    if (parts.size() != 1 && parts.size() != 2) {
        return false;
    }

    try {
        std::vector<int> values;
        for (const auto& part : parts) {
            std::size_t parsed = 0;
            const auto input = NeoWPF::Platform::Trim(part);
            values.push_back(std::stoi(input, &parsed));
            if (parsed != input.size()) {
                return false;
            }
        }

        convertedValue = values.size() == 1
            ? NeoWPF::UI::Spacing(values[0])
            : NeoWPF::UI::Spacing(values[0], values[1]);
        return true;
    } catch (...) {
        return false;
    }
}

bool HorizontalAlignmentConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value == "left") convertedValue = NeoWPF::UI::HorizontalAlignment::Left;
    else if (value == "center") convertedValue = NeoWPF::UI::HorizontalAlignment::Center;
    else if (value == "right") convertedValue = NeoWPF::UI::HorizontalAlignment::Right;
    else if (value == "stretch") convertedValue = NeoWPF::UI::HorizontalAlignment::Stretch;
    else return false;
    return true;
}

bool VerticalAlignmentConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value == "top") convertedValue = NeoWPF::UI::VerticalAlignment::Top;
    else if (value == "center") convertedValue = NeoWPF::UI::VerticalAlignment::Center;
    else if (value == "bottom") convertedValue = NeoWPF::UI::VerticalAlignment::Bottom;
    else if (value == "stretch") convertedValue = NeoWPF::UI::VerticalAlignment::Stretch;
    else return false;
    return true;
}

#define DEFINE_CONVERTER_FACTORY(Name, Type) \
std::shared_ptr<const TypeConverter> Get##Name##Converter() \
{ \
    static const auto converter = std::make_shared<Type>(); \
    return converter; \
}

DEFINE_CONVERTER_FACTORY(String, StringConverter)
DEFINE_CONVERTER_FACTORY(WideString, WideStringConverter)
DEFINE_CONVERTER_FACTORY(Int, IntConverter)
DEFINE_CONVERTER_FACTORY(Float, FloatConverter)
DEFINE_CONVERTER_FACTORY(Bool, BoolConverter)
DEFINE_CONVERTER_FACTORY(Color, ColorConverter)
DEFINE_CONVERTER_FACTORY(Thickness, ThicknessConverter)
DEFINE_CONVERTER_FACTORY(Spacing, SpacingConverter)
DEFINE_CONVERTER_FACTORY(HorizontalAlignment, HorizontalAlignmentConverter)
DEFINE_CONVERTER_FACTORY(VerticalAlignment, VerticalAlignmentConverter)

#undef DEFINE_CONVERTER_FACTORY

bool TryConvertValue(const DependencyProperty& property,
                     const std::any& value,
                     std::any& convertedValue)
{
    if (!value.has_value()) {
        convertedValue.reset();
        return true;
    }

    if (value.type() == property.GetPropertyType()) {
        convertedValue = value;
        return true;
    }

    std::string text;
    if (value.type() == typeid(std::wstring)) {
        text = NeoWPF::Platform::WideToUtf8(std::any_cast<std::wstring>(value));
    } else if (value.type() == typeid(std::string)) {
        text = std::any_cast<std::string>(value);
    } else {
        return false;
    }

    if (!property.TryParseValue(text, convertedValue) ||
        convertedValue.type() != property.GetPropertyType()) {
        convertedValue.reset();
        return false;
    }

    return true;
}

}
