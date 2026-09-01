#pragma once

#include <any>
#include <string>
#include <typeinfo>

namespace NeoWPF::UI::Properties {

class TypeConverter {
public:
    virtual ~TypeConverter() = default;

    virtual bool CanConvertFrom(const std::type_info& sourceType) const;
    virtual bool CanConvertFromString(const std::string& value) const;
    virtual bool ConvertFromString(const std::string& rawValue, std::any& convertedValue) const = 0;

    virtual std::string ConvertToString(const std::any& value) const;
};

}
