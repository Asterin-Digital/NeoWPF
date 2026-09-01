#include <NeoWPF/UI/Properties/TypeConverter.h>
#include <NeoWPF/Platform/PlatformUtils.h>

namespace NeoWPF::UI::Properties {

bool TypeConverter::CanConvertFrom(const std::type_info& sourceType) const {
    return sourceType == typeid(std::string);
}

bool TypeConverter::CanConvertFromString(const std::string& value) const {
    return !NeoWPF::Platform::Trim(value).empty();
}

std::string TypeConverter::ConvertToString(const std::any& value) const {
    if (value.type() == typeid(std::string)) {
        return std::any_cast<std::string>(value);
    }
    if (value.type() == typeid(std::wstring)) {
        return NeoWPF::Platform::WideToUtf8(std::any_cast<std::wstring>(value));
    }
    return {};
}

}
