#include <NeoWPF/UI/Localization/LocaleString.h>

#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {
NeoWPF::UI::Properties::PropertyMetadata MakeStringMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(std::wstring());
    metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
    return metadata;
}
}

namespace NeoWPF::UI::Properties {
namespace UI = ::NeoWPF::UI;

void RegisterLocaleStringProperties()
{
    RegisterProperty<UI::Localization::LocaleString, std::wstring>(
        "Key",
        &UI::Localization::LocaleString::GetKey,
        static_cast<void (UI::Localization::LocaleString::*)(const std::wstring&)>(
            &UI::Localization::LocaleString::SetKey),
        MakeStringMetadata());
    RegisterProperty<UI::Localization::LocaleString, std::wstring>(
        "Value",
        &UI::Localization::LocaleString::GetValue,
        static_cast<void (UI::Localization::LocaleString::*)(const std::wstring&)>(
            &UI::Localization::LocaleString::SetValue),
        MakeStringMetadata());
    UI::Markup::Xaml::RegisterXamlType<UI::Localization::LocaleString>(
        "LocaleString", "", "NeoWPF::UI::Localization::LocaleString",
        "NeoWPF/UI/Localization/LocaleString.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty&
NeoWPF::UI::Localization::LocaleString::KeyProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Localization::LocaleString, std::wstring>(
        "Key",
        &NeoWPF::UI::Localization::LocaleString::GetKey,
        static_cast<void (NeoWPF::UI::Localization::LocaleString::*)(const std::wstring&)>(
            &NeoWPF::UI::Localization::LocaleString::SetKey),
        MakeStringMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty&
NeoWPF::UI::Localization::LocaleString::ValueProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Localization::LocaleString, std::wstring>(
        "Value",
        &NeoWPF::UI::Localization::LocaleString::GetValue,
        static_cast<void (NeoWPF::UI::Localization::LocaleString::*)(const std::wstring&)>(
            &NeoWPF::UI::Localization::LocaleString::SetValue),
        MakeStringMetadata());
    return *property;
}
