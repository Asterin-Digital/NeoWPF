#include <NeoWPF/UI/Controls/Base/GroupedToggleBase/GroupedToggleBase.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterGroupedToggleBaseProperties()
{
    PropertyMetadata metadata;
    metadata.DefaultValue = std::any(std::wstring());
    metadata.Converter = GetWideStringConverter();
    RegisterProperty<UI::GroupedToggleBase, std::wstring>(
        "GroupName", &UI::GroupedToggleBase::GetGroupName, &UI::GroupedToggleBase::SetGroupName, metadata);
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::GroupedToggleBase::GroupNameProperty()
{
    static const auto& property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::GroupedToggleBase, std::wstring>(
            "GroupName", &NeoWPF::UI::GroupedToggleBase::GetGroupName,
            &NeoWPF::UI::GroupedToggleBase::SetGroupName);
    return *property;
}