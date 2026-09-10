#include <NeoWPF/UI/Localization/LocaleTable.h>

#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {
NeoWPF::UI::Properties::PropertyMetadata MakeCultureMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(std::wstring());
    metadata.Converter = NeoWPF::UI::Properties::GetWideStringConverter();
    return metadata;
}
}

namespace NeoWPF::UI::Properties {
namespace UI = ::NeoWPF::UI;

void RegisterLocaleTableProperties()
{
    RegisterProperty<UI::Localization::LocaleTable, std::wstring>(
        "Culture",
        [](UI::Localization::LocaleTable& table) { return table.GetCulture().GetName(); },
        &UI::Localization::LocaleTable::SetCulture,
        MakeCultureMetadata());
    (void)DependencyProperty::Register<
        UI::Localization::LocaleTable,
        std::vector<std::shared_ptr<UI::Localization::LocaleString>>>("Strings", {});
    UI::Markup::Xaml::RegisterXamlCollectionProperty(
        UI::Localization::LocaleTable::StringsProperty(),
        &UI::Localization::LocaleTable::AddLocaleString);
    UI::Markup::Xaml::RegisterXamlType<UI::Localization::LocaleTable>(
        "LocaleTable", "Strings", "NeoWPF::UI::Localization::LocaleTable",
        "NeoWPF/UI/Localization/LocaleTable.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty&
NeoWPF::UI::Localization::LocaleTable::CultureProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<
        NeoWPF::UI::Localization::LocaleTable, std::wstring>("Culture", MakeCultureMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty&
NeoWPF::UI::Localization::LocaleTable::StringsProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::Register<
        NeoWPF::UI::Localization::LocaleTable,
        std::vector<std::shared_ptr<NeoWPF::UI::Localization::LocaleString>>>(
        "Strings",
        {});
    return property;
}

void NeoWPF::UI::Localization::LocaleTable::SetCulture(const std::wstring& culture)
{
    m_culture = Culture(culture);
}

void NeoWPF::UI::Localization::LocaleTable::AddLocaleString(
    std::shared_ptr<LocaleString> value)
{
    if (!value || !Add(value->GetKey(), value->GetValue())) {
        return;
    }
    m_localeStrings.push_back(std::move(value));
}