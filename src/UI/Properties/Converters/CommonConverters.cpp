#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Core/Media/Thickness.h>
#include <NeoWPF/UI/Core/Media/Spacing.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <NeoWPF/UI/Controls/Selector/Selector.h>
#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Text/FontStyle.h>

#include <cstdint>
#include <array>
#include <cmath>
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

std::string FormatCodeNumber(double value)
{
    std::ostringstream result;
    result << std::setprecision(15) << value;
    return result.str();
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

template<typename T>
struct ConversionEntry {
    const char* name;
    T value;
    const char* code;
};

template<typename T, std::size_t Count>
bool ConvertMappedValue(const std::string& rawValue,
                        const std::array<ConversionEntry<T>, Count>& entries,
                        std::any& convertedValue)
{
    const auto value = NeoWPF::Platform::ToLower(NeoWPF::Platform::Trim(rawValue));
    for (const auto& entry : entries) {
        if (value == entry.name) {
            convertedValue = entry.value;
            return true;
        }
    }
    return false;
}

template<typename T, std::size_t Count>
bool ConvertMappedValueToCode(const TypeConverter& converter,
                              const std::string& rawValue,
                              const std::array<ConversionEntry<T>, Count>& entries,
                              std::string& expression)
{
    std::any convertedValue;
    if (!converter.ConvertFromString(rawValue, convertedValue)) {
        expression.clear();
        return false;
    }

    const auto value = std::any_cast<T>(convertedValue);
    for (const auto& entry : entries) {
        if (value == entry.value) {
            expression = entry.code;
            return true;
        }
    }

    expression.clear();
    return false;
}

constexpr std::array<ConversionEntry<NeoWPF::UI::HorizontalAlignment>, 4> HorizontalAlignmentEntries {{
    {"left", NeoWPF::UI::HorizontalAlignment::Left, "NeoWPF::UI::HorizontalAlignment::Left"},
    {"center", NeoWPF::UI::HorizontalAlignment::Center, "NeoWPF::UI::HorizontalAlignment::Center"},
    {"right", NeoWPF::UI::HorizontalAlignment::Right, "NeoWPF::UI::HorizontalAlignment::Right"},
    {"stretch", NeoWPF::UI::HorizontalAlignment::Stretch, "NeoWPF::UI::HorizontalAlignment::Stretch"},
}};

constexpr std::array<ConversionEntry<NeoWPF::UI::VerticalAlignment>, 4> VerticalAlignmentEntries {{
    {"top", NeoWPF::UI::VerticalAlignment::Top, "NeoWPF::UI::VerticalAlignment::Top"},
    {"center", NeoWPF::UI::VerticalAlignment::Center, "NeoWPF::UI::VerticalAlignment::Center"},
    {"bottom", NeoWPF::UI::VerticalAlignment::Bottom, "NeoWPF::UI::VerticalAlignment::Bottom"},
    {"stretch", NeoWPF::UI::VerticalAlignment::Stretch, "NeoWPF::UI::VerticalAlignment::Stretch"},
}};

constexpr std::array<ConversionEntry<NeoWPF::UI::SelectionMode>, 3> SelectionModeEntries {{
    {"single", NeoWPF::UI::SelectionMode::Single, "NeoWPF::UI::SelectionMode::Single"},
    {"multiple", NeoWPF::UI::SelectionMode::Multiple, "NeoWPF::UI::SelectionMode::Multiple"},
    {"extended", NeoWPF::UI::SelectionMode::Extended, "NeoWPF::UI::SelectionMode::Extended"},
}};

constexpr std::array<ConversionEntry<NeoWPF::UI::PopupPlacement>, 4> PopupPlacementEntries {{
    {"bottom", NeoWPF::UI::PopupPlacement::Bottom, "NeoWPF::UI::PopupPlacement::Bottom"},
    {"top", NeoWPF::UI::PopupPlacement::Top, "NeoWPF::UI::PopupPlacement::Top"},
    {"left", NeoWPF::UI::PopupPlacement::Left, "NeoWPF::UI::PopupPlacement::Left"},
    {"right", NeoWPF::UI::PopupPlacement::Right, "NeoWPF::UI::PopupPlacement::Right"},
}};

constexpr std::array<ConversionEntry<NeoWPF::UI::FontStyle>, 3> FontStyleEntries {{
    {"normal", NeoWPF::UI::FontStyle::Normal, "NeoWPF::UI::FontStyle::Normal"},
    {"italic", NeoWPF::UI::FontStyle::Italic, "NeoWPF::UI::FontStyle::Italic"},
    {"oblique", NeoWPF::UI::FontStyle::Oblique, "NeoWPF::UI::FontStyle::Oblique"},
}};

constexpr std::array<ConversionEntry<int>, 9> FontWeightEntries {{
    {"thin", 100, "100"},
    {"extralight", 200, "200"},
    {"light", 300, "300"},
    {"normal", 400, "400"},
    {"medium", 500, "500"},
    {"semibold", 600, "600"},
    {"bold", 700, "700"},
    {"extrabold", 800, "800"},
    {"black", 900, "900"},
}};

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

bool ContentConverter::TryConvertToText(const std::any& value, std::wstring& text)
{
    if (const auto* stringValue = std::any_cast<const wchar_t*>(&value)) {
        if (*stringValue) {
            text = *stringValue;
            return true;
        }
        return false;
    }
    if (const auto* stringValue = std::any_cast<wchar_t*>(&value)) {
        if (*stringValue) {
            text = *stringValue;
            return true;
        }
        return false;
    }
    if (const auto* stringValue = std::any_cast<std::wstring>(&value)) {
        text = *stringValue;
        return true;
    }
    if (const auto* stringValue = std::any_cast<std::string>(&value)) {
        text = NeoWPF::Platform::Utf8ToWide(*stringValue);
        return true;
    }
    if (const auto* number = std::any_cast<int>(&value)) {
        text = std::to_wstring(*number);
        return true;
    }
    if (const auto* number = std::any_cast<double>(&value)) {
        text = std::to_wstring(*number);
        return true;
    }
    if (const auto* boolean = std::any_cast<bool>(&value)) {
        text = *boolean ? L"True" : L"False";
        return true;
    }
    return false;
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

bool DoubleConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    try {
        std::size_t parsed = 0;
        const auto input = NeoWPF::Platform::Trim(rawValue);
        const double value = std::stod(input, &parsed);
        if (parsed != input.size()) {
            return false;
        }
        convertedValue = value;
        return true;
    } catch (...) {
        return false;
    }
}

bool DoubleConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ParseAndFormat(*this, rawValue, expression, [](const std::any& value) {
        return std::to_string(std::any_cast<double>(value));
    });
}

bool FloatConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    try {
        const auto input = NeoWPF::Platform::Trim(rawValue);
        if (input.empty() || input.back() == 'f' || input.back() == 'F') {
            return false;
        }

        std::size_t parsed = 0;
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
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(6) << std::any_cast<float>(value) << 'f';
        return stream.str();
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
        std::vector<double> values;
        for (const auto& part : parts) {
            std::size_t parsed = 0;
            const auto input = NeoWPF::Platform::Trim(part);
            values.push_back(std::stod(input, &parsed));
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
        FormatCodeNumber(thickness.Left) + ", " +
        FormatCodeNumber(thickness.Top) + ", " +
        FormatCodeNumber(thickness.Right) + ", " +
        FormatCodeNumber(thickness.Bottom) + ")";
    return true;
}

bool SpacingConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    const auto parts = NeoWPF::Platform::Split(NeoWPF::Platform::Trim(rawValue), ',');
    if (parts.size() != 1 && parts.size() != 2) {
        return false;
    }

    try {
        std::vector<double> values;
        for (const auto& part : parts) {
            std::size_t parsed = 0;
            const auto input = NeoWPF::Platform::Trim(part);
            values.push_back(std::stod(input, &parsed));
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
    return ConvertMappedValue(rawValue, HorizontalAlignmentEntries, convertedValue);
}

bool HorizontalAlignmentConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ConvertMappedValueToCode(*this, rawValue, HorizontalAlignmentEntries, expression);
}

bool VerticalAlignmentConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    return ConvertMappedValue(rawValue, VerticalAlignmentEntries, convertedValue);
}

bool VerticalAlignmentConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ConvertMappedValueToCode(*this, rawValue, VerticalAlignmentEntries, expression);
}

bool SelectionModeConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    return ConvertMappedValue(rawValue, SelectionModeEntries, convertedValue);
}

bool SelectionModeConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ConvertMappedValueToCode(*this, rawValue, SelectionModeEntries, expression);
}

bool PopupPlacementConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    return ConvertMappedValue(rawValue, PopupPlacementEntries, convertedValue);
}

bool PopupPlacementConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ConvertMappedValueToCode(*this, rawValue, PopupPlacementEntries, expression);
}

bool FontWeightConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    return ConvertMappedValue(rawValue, FontWeightEntries, convertedValue);
}

bool FontWeightConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ConvertMappedValueToCode(*this, rawValue, FontWeightEntries, expression);
}

bool FontStyleConverter::ConvertFromString(const std::string& rawValue, std::any& convertedValue) const
{
    return ConvertMappedValue(rawValue, FontStyleEntries, convertedValue);
}

bool FontStyleConverter::TryConvertToCode(const std::string& rawValue, std::string& expression) const
{
    return ConvertMappedValueToCode(*this, rawValue, FontStyleEntries, expression);
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
DEFINE_CONVERTER_FACTORY(Double, DoubleConverter)
DEFINE_CONVERTER_FACTORY(Float, FloatConverter)
DEFINE_CONVERTER_FACTORY(Bool, BoolConverter)
DEFINE_CONVERTER_FACTORY(Color, ColorConverter)
DEFINE_CONVERTER_FACTORY(Thickness, ThicknessConverter)
DEFINE_CONVERTER_FACTORY(Spacing, SpacingConverter)
DEFINE_CONVERTER_FACTORY(HorizontalAlignment, HorizontalAlignmentConverter)
DEFINE_CONVERTER_FACTORY(VerticalAlignment, VerticalAlignmentConverter)
DEFINE_CONVERTER_FACTORY(SelectionMode, SelectionModeConverter)
DEFINE_CONVERTER_FACTORY(PopupPlacement, PopupPlacementConverter)
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
