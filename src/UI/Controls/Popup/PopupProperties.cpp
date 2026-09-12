#include <NeoWPF/UI/Controls/Popup/Popup.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace {

NeoWPF::UI::Properties::PropertyMetadata MakeBoolMetadata(bool defaultValue)
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(defaultValue);
    metadata.Converter = NeoWPF::UI::Properties::GetBoolConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

NeoWPF::UI::Properties::PropertyMetadata MakeOffsetMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(0);
    metadata.Converter = NeoWPF::UI::Properties::GetIntConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsArrange |
                     NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

NeoWPF::UI::Properties::PropertyMetadata MakePlacementMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(NeoWPF::UI::PopupPlacement::Bottom);
    metadata.Converter = NeoWPF::UI::Properties::GetPopupPlacementConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsArrange |
                     NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
    return metadata;
}

}

namespace NeoWPF::UI::Properties {

void RegisterPopupProperties()
{
    RegisterProperty<UI::Popup, bool>("IsOpen",
        &UI::Popup::GetIsOpen, &UI::Popup::SetIsOpen, MakeBoolMetadata(false));
    RegisterProperty<UI::Popup, bool>("StaysOpen",
        &UI::Popup::GetStaysOpen, &UI::Popup::SetStaysOpen, MakeBoolMetadata(true));
    RegisterProperty<UI::Popup, UI::FrameworkElement*>("PlacementTarget",
        &UI::Popup::GetPlacementTarget, &UI::Popup::SetPlacementTarget);
    RegisterProperty<UI::Popup, UI::PopupPlacement>("Placement",
        &UI::Popup::GetPlacement, &UI::Popup::SetPlacement, MakePlacementMetadata());
    RegisterProperty<UI::Popup, int>("HorizontalOffset",
        &UI::Popup::GetHorizontalOffset, &UI::Popup::SetHorizontalOffset, MakeOffsetMetadata());
    RegisterProperty<UI::Popup, int>("VerticalOffset",
        &UI::Popup::GetVerticalOffset, &UI::Popup::SetVerticalOffset, MakeOffsetMetadata());

    NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::Popup>(
        "Popup", "Content", "NeoWPF::UI::Popup", "NeoWPF/UI/Controls/Popup/Popup.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Popup::IsOpenProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Popup, bool>("IsOpen", &NeoWPF::UI::Popup::GetIsOpen,
            &NeoWPF::UI::Popup::SetIsOpen, MakeBoolMetadata(false));
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Popup::StaysOpenProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Popup, bool>("StaysOpen", &NeoWPF::UI::Popup::GetStaysOpen,
            &NeoWPF::UI::Popup::SetStaysOpen, MakeBoolMetadata(true));
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Popup::PlacementTargetProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Popup, NeoWPF::UI::FrameworkElement*>("PlacementTarget",
            &NeoWPF::UI::Popup::GetPlacementTarget, &NeoWPF::UI::Popup::SetPlacementTarget);
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Popup::PlacementProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Popup, NeoWPF::UI::PopupPlacement>("Placement", &NeoWPF::UI::Popup::GetPlacement,
            &NeoWPF::UI::Popup::SetPlacement, MakePlacementMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Popup::HorizontalOffsetProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Popup, int>("HorizontalOffset", &NeoWPF::UI::Popup::GetHorizontalOffset,
            &NeoWPF::UI::Popup::SetHorizontalOffset, MakeOffsetMetadata());
    return *property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Popup::VerticalOffsetProperty()
{
    static const auto* property = NeoWPF::UI::Properties::RegisterStaticPropertyHandle<
        NeoWPF::UI::Popup, int>("VerticalOffset", &NeoWPF::UI::Popup::GetVerticalOffset,
            &NeoWPF::UI::Popup::SetVerticalOffset, MakeOffsetMetadata());
    return *property;
}
