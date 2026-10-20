/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/TypeConverter.h>

#include <any>
#include <memory>
#include <typeindex>

namespace NeoWPF::UI {
enum class PopupPlacement;
enum class LayoutSizeMode;
enum class Orientation;
enum class TextWrapping;
enum class TextHorizontalAlignment;
enum class TextVerticalAlignment;
struct CornerRadius;
}

namespace NeoWPF::UI::Properties {

class DependencyProperty;

class StringConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class ContentConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
    static bool TryConvertToText(const std::any& value, std::wstring& text);
};

class WideStringConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class IntConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class DoubleConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class FloatConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class BoolConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class BrushConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class ThicknessConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class SpacingConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class CornerRadiusConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class TextWrappingConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class LayoutSizeModeConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class OrientationConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class HorizontalAlignmentConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class VerticalAlignmentConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class TextHorizontalAlignmentConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class TextVerticalAlignmentConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class SelectionModeConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class PopupPlacementConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class FontWeightConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class FontStyleConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

class TypeIndexConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
};

std::shared_ptr<const TypeConverter> GetStringConverter();
std::shared_ptr<const TypeConverter> GetContentConverter();
std::shared_ptr<const TypeConverter> GetWideStringConverter();
std::shared_ptr<const TypeConverter> GetIntConverter();
std::shared_ptr<const TypeConverter> GetDoubleConverter();
std::shared_ptr<const TypeConverter> GetFloatConverter();
std::shared_ptr<const TypeConverter> GetBoolConverter();
std::shared_ptr<const TypeConverter> GetBrushConverter();
std::shared_ptr<const TypeConverter> GetThicknessConverter();
std::shared_ptr<const TypeConverter> GetSpacingConverter();
std::shared_ptr<const TypeConverter> GetCornerRadiusConverter();
std::shared_ptr<const TypeConverter> GetTextWrappingConverter();
std::shared_ptr<const TypeConverter> GetLayoutSizeModeConverter();
std::shared_ptr<const TypeConverter> GetOrientationConverter();
std::shared_ptr<const TypeConverter> GetHorizontalAlignmentConverter();
std::shared_ptr<const TypeConverter> GetVerticalAlignmentConverter();
std::shared_ptr<const TypeConverter> GetTextHorizontalAlignmentConverter();
std::shared_ptr<const TypeConverter> GetTextVerticalAlignmentConverter();
std::shared_ptr<const TypeConverter> GetSelectionModeConverter();
std::shared_ptr<const TypeConverter> GetPopupPlacementConverter();
std::shared_ptr<const TypeConverter> GetFontWeightConverter();
std::shared_ptr<const TypeConverter> GetFontStyleConverter();
std::shared_ptr<const TypeConverter> GetTypeIndexConverter();

bool TryConvertValue(const DependencyProperty& property,
                     const std::any& value,
                     std::any& convertedValue);

}
