#pragma once

#include <NeoWPF/UI/Properties/TypeConverter.h>

#include <memory>

namespace NeoWPF::UI::Properties {

class StringConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class WideStringConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class IntConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class FloatConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class BoolConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class ColorConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class ThicknessConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class HorizontalAlignmentConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

class VerticalAlignmentConverter final : public TypeConverter {
public:
    bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const override;
};

std::shared_ptr<const TypeConverter> GetStringConverter();
std::shared_ptr<const TypeConverter> GetWideStringConverter();
std::shared_ptr<const TypeConverter> GetIntConverter();
std::shared_ptr<const TypeConverter> GetFloatConverter();
std::shared_ptr<const TypeConverter> GetBoolConverter();
std::shared_ptr<const TypeConverter> GetColorConverter();
std::shared_ptr<const TypeConverter> GetThicknessConverter();
std::shared_ptr<const TypeConverter> GetHorizontalAlignmentConverter();
std::shared_ptr<const TypeConverter> GetVerticalAlignmentConverter();

}
