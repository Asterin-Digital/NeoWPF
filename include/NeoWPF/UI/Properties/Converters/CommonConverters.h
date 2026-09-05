#pragma once

#include <NeoWPF/UI/Properties/TypeConverter.h>

#include <any>
#include <memory>

namespace NeoWPF::UI::Properties {

class DependencyProperty;

class StringConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
    bool TryConvertToCode(const std::string& rawValue, std::string& expression) const override;
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

class ColorConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class ThicknessConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class SpacingConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
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

std::shared_ptr<const TypeConverter> GetStringConverter();
std::shared_ptr<const TypeConverter> GetWideStringConverter();
std::shared_ptr<const TypeConverter> GetIntConverter();
std::shared_ptr<const TypeConverter> GetFloatConverter();
std::shared_ptr<const TypeConverter> GetBoolConverter();
std::shared_ptr<const TypeConverter> GetColorConverter();
std::shared_ptr<const TypeConverter> GetThicknessConverter();
std::shared_ptr<const TypeConverter> GetSpacingConverter();
std::shared_ptr<const TypeConverter> GetHorizontalAlignmentConverter();
std::shared_ptr<const TypeConverter> GetVerticalAlignmentConverter();

bool TryConvertValue(const DependencyProperty& property,
                     const std::any& value,
                     std::any& convertedValue);

}
