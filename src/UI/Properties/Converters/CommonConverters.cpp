#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/Thickness.h>
#include <NeoWPF/UI/Core/Media/Spacing.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <NeoWPF/UI/Text/FontStyle.h>

#include <cctype>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace NeoWPF::UI::Properties {

namespace {

std::string EscapeLiteral(const std::string& value, bool wide)
{
    std::ostringstream result;
    if (wide) {
        result << 'L';
    }
    result << '"';
    for (const char character : value) {
        switch (character) {
        case '\\': result << "\\\\"; break;
        case '"': result << "\\\""; break;
        case '\n': result << "\\n"; break;
        case '\r': result << "\\r"; break;
        case '\t': result << "\\t"; break;
        default:
            if (static_cast<unsigned char>(character) >= 0x20 &&
                static_cast<unsigned char>(character) <= 0x7e) {
                result << character;
            } else {
                result << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                       << static_cast<unsigned int>(static_cast<unsigned char>(character)) << std::dec;
            }
            break;
        }
    }
    result << '"';
    return result.str();
}

std::string UppercaseFirst(std::string value)
{
    if (!value.empty()) {
        value.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(value.front())));
    }
    return value;
}

template<typename Formatter>
bool ParseAndFormat(const TypeConverter& converter,
                    const std::string& rawValue,
                    std::string& expression,
                    Formatter formatter)
{
    expression.clear();
    std::any parsedValue;
    if (!converter.ConvertFromString(rawValue, parsedValue)) {
        return false;
    }
    expression = formatter(parsedValue);
    return !expression.empty();
}

}

bool StringConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    convertedValue = rawValue;
    return true;
}

bool StringConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    expression = EscapeLiteral(rawValue, false);
    return true;
}

bool ContentConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    std::shared_ptr<NeoWPF::UI::UIElement> textElement =
        std::make_shared<NeoWPF::UI::TextBlock>(NeoWPF::Platform::Utf8ToWide(rawValue));
    convertedValue = std::move(textElement);
    return true;
}

bool ContentConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    expression = "std::string(" + EscapeLiteral(rawValue, false) + ")";
    return true;
}

bool WideStringConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    convertedValue = std::wstring(rawValue.begin(), rawValue.end());
    return true;
}

bool WideStringConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    expression = "std::wstring(" + EscapeLiteral(rawValue, true) + ")";
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

bool IntConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ParseAndFormat(*this, rawValue, expression, [](const std::any& value) {
        return std::to_string(std::any_cast<int>(value));
    });
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

bool FloatConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ParseAndFormat(*this, rawValue, expression, [](const std::any& value) {
        return std::to_string(std::any_cast<float>(value)) + "f";
    });
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

bool BoolConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ParseAndFormat(*this, rawValue, expression, [](const std::any& value) {
        return std::any_cast<bool>(value) ? std::string("true") : std::string("false");
    });
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

bool ThicknessConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    std::any convertedValue;
    if (!ConvertFromString(rawValue, convertedValue)) {
        expression.clear();
        return false;
    }

    const auto thickness = std::any_cast<NeoWPF::UI::Thickness>(convertedValue);
    expression = "NeoWPF::UI::Thickness(" +
        std::to_string(thickness.Left) + ", " +
        std::to_string(thickness.Top) + ", " +
        std::to_string(thickness.Right) + ", " +
        std::to_string(thickness.Bottom) + ")";
    return true;
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

bool HorizontalAlignmentConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    expression.clear();
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value != "left" && value != "center" && value != "right" && value != "stretch") {
        return false;
    }
    expression = "NeoWPF::UI::HorizontalAlignment::";
    expression += UppercaseFirst(value);
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

bool VerticalAlignmentConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    expression.clear();
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value != "top" && value != "center" && value != "bottom" && value != "stretch") {
        return false;
    }
    expression = "NeoWPF::UI::VerticalAlignment::";
    expression += UppercaseFirst(value);
    return true;
}

bool SelectionModeConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value == "single") {
        convertedValue = NeoWPF::UI::SelectionMode::Single;
        return true;
    }
    if (value == "multiple") {
        convertedValue = NeoWPF::UI::SelectionMode::Multiple;
        return true;
    }
    if (value == "extended") {
        convertedValue = NeoWPF::UI::SelectionMode::Extended;
        return true;
    }
    return false;
}

bool SelectionModeConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    std::any convertedValue;
    if (!ConvertFromString(rawValue, convertedValue)) {
        expression.clear();
        return false;
    }

    const auto mode = std::any_cast<NeoWPF::UI::SelectionMode>(convertedValue);
    switch (mode) {
    case NeoWPF::UI::SelectionMode::Single: expression = "NeoWPF::UI::SelectionMode::Single"; break;
    case NeoWPF::UI::SelectionMode::Multiple: expression = "NeoWPF::UI::SelectionMode::Multiple"; break;
    case NeoWPF::UI::SelectionMode::Extended: expression = "NeoWPF::UI::SelectionMode::Extended"; break;
    }
    return true;
}

bool FontWeightConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    
    if (value == "thin") convertedValue = 100;
    else if (value == "extralight") convertedValue = 200;
    else if (value == "light") convertedValue = 300;
    else if (value == "normal") convertedValue = 400;
    else if (value == "medium") convertedValue = 500;
    else if (value == "semibold") convertedValue = 600;
    else if (value == "bold") convertedValue = 700;
    else if (value == "extrabold") convertedValue = 800;
    else if (value == "black") convertedValue = 900;
    else return false;
    return true;
}

bool FontWeightConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    std::any convertedValue;
    if (!ConvertFromString(rawValue, convertedValue)) {
        expression.clear();
        return false;
    }

    const auto weight = std::any_cast<int>(convertedValue);
    expression = std::to_string(weight);
    return true;
}

bool FontStyleConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    if (value == "normal") convertedValue = NeoWPF::UI::FontStyle::Normal;
    else if (value == "italic") convertedValue = NeoWPF::UI::FontStyle::Italic;
    else if (value == "oblique") convertedValue = NeoWPF::UI::FontStyle::Oblique;
    else return false;
    return true;
}

bool FontStyleConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    std::any convertedValue;
    if (!ConvertFromString(rawValue, convertedValue)) {
        expression.clear();
        return false;
    }

    const auto style = std::any_cast<NeoWPF::UI::FontStyle>(convertedValue);
    switch (style) {
    case NeoWPF::UI::FontStyle::Normal: expression = "NeoWPF::UI::FontStyle::Normal"; break;
    case NeoWPF::UI::FontStyle::Italic: expression = "NeoWPF::UI::FontStyle::Italic"; break;
    case NeoWPF::UI::FontStyle::Oblique: expression = "NeoWPF::UI::FontStyle::Oblique"; break;
    }
    return true;
}

#define DEFINE_CONVERTER_FACTORY(Name, Type) \
std::shared_ptr<const TypeConverter> Get##Name##Converter() \
{ \
    static const auto converter = std::make_shared<Type>(); \
    return converter; \
}

DEFINE_CONVERTER_FACTORY(String, StringConverter)
DEFINE_CONVERTER_FACTORY(Content, ContentConverter)
DEFINE_CONVERTER_FACTORY(WideString, WideStringConverter)
DEFINE_CONVERTER_FACTORY(Int, IntConverter)
DEFINE_CONVERTER_FACTORY(Float, FloatConverter)
DEFINE_CONVERTER_FACTORY(Bool, BoolConverter)
DEFINE_CONVERTER_FACTORY(Color, ColorConverter)
DEFINE_CONVERTER_FACTORY(Thickness, ThicknessConverter)
DEFINE_CONVERTER_FACTORY(Spacing, SpacingConverter)
DEFINE_CONVERTER_FACTORY(HorizontalAlignment, HorizontalAlignmentConverter)
DEFINE_CONVERTER_FACTORY(VerticalAlignment, VerticalAlignmentConverter)
DEFINE_CONVERTER_FACTORY(SelectionMode, SelectionModeConverter)
DEFINE_CONVERTER_FACTORY(FontWeight, FontWeightConverter)
DEFINE_CONVERTER_FACTORY(FontStyle, FontStyleConverter)

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

    if (property.GetPropertyType() == typeid(std::any) &&
        value.type() != typeid(std::wstring) &&
        value.type() != typeid(std::string)) {
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
        (property.GetPropertyType() != typeid(std::any) &&
         convertedValue.type() != property.GetPropertyType())) {
        convertedValue.reset();
        return false;
    }

    return true;
}

}
