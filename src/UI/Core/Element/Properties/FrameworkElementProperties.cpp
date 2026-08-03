#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterFrameworkElementProperties() {
        PropertyMetadata widthMetadata;
        widthMetadata.DefaultValue = std::any(0);
        widthMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;

        RegisterProperty<UI::FrameworkElement, int>(
            "Width",
            &UI::FrameworkElement::GetWidth,
            &UI::FrameworkElement::SetWidth,
            widthMetadata);

        PropertyMetadata heightMetadata;
        heightMetadata.DefaultValue = std::any(0);
        heightMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;

        RegisterProperty<UI::FrameworkElement, int>(
            "Height",
            &UI::FrameworkElement::GetHeight,
            &UI::FrameworkElement::SetHeight,
            heightMetadata);

        PropertyMetadata widthModeMetadata;
        widthModeMetadata.DefaultValue = std::any(UI::LayoutSizeMode::Auto);
        widthModeMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;

        RegisterProperty<UI::FrameworkElement, UI::LayoutSizeMode>(
            "WidthMode",
            &UI::FrameworkElement::GetWidthMode,
            &UI::FrameworkElement::SetWidthMode,
            widthModeMetadata);

        PropertyMetadata heightModeMetadata;
        heightModeMetadata.DefaultValue = std::any(UI::LayoutSizeMode::Auto);
        heightModeMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;

        RegisterProperty<UI::FrameworkElement, UI::LayoutSizeMode>(
            "HeightMode",
            &UI::FrameworkElement::GetHeightMode,
            &UI::FrameworkElement::SetHeightMode,
            heightModeMetadata);

        PropertyMetadata clipToBoundsMetadata;
        clipToBoundsMetadata.DefaultValue = std::any(false);
        clipToBoundsMetadata.Flags = PropertyFlags::AffectsRender;

        RegisterProperty<UI::FrameworkElement, bool>(
            "ClipToBounds",
            &UI::FrameworkElement::IsClipToBounds,
            &UI::FrameworkElement::SetClipToBounds,
            clipToBoundsMetadata);

        PropertyMetadata horizontalAlignmentMetadata;
        horizontalAlignmentMetadata.DefaultValue = std::any(UI::HorizontalAlignment::Stretch);
        horizontalAlignmentMetadata.Flags = PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;

        RegisterProperty<UI::FrameworkElement, UI::HorizontalAlignment>(
            "HorizontalAlignment",
            &UI::FrameworkElement::GetHorizontalAlignment,
            &UI::FrameworkElement::SetHorizontalAlignment,
            horizontalAlignmentMetadata);

        PropertyMetadata verticalAlignmentMetadata;
        verticalAlignmentMetadata.DefaultValue = std::any(UI::VerticalAlignment::Stretch);
        verticalAlignmentMetadata.Flags = PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;

        RegisterProperty<UI::FrameworkElement, UI::VerticalAlignment>(
            "VerticalAlignment",
            &UI::FrameworkElement::GetVerticalAlignment,
            &UI::FrameworkElement::SetVerticalAlignment,
            verticalAlignmentMetadata);

        PropertyMetadata marginMetadata;
        marginMetadata.DefaultValue = std::any(UI::Thickness{});
        marginMetadata.Flags = PropertyFlags::AffectsMeasure | PropertyFlags::AffectsArrange | PropertyFlags::AffectsRender;

        RegisterProperty<UI::FrameworkElement, UI::Thickness>(
            "Margin",
            &UI::FrameworkElement::GetMargin,
            [](UI::FrameworkElement* owner, const UI::Thickness& value) {
                if (owner) {
                    owner->SetMargin(value);
                }
            },
            marginMetadata);

        PropertyMetadata cornerRadiusMetadata;
        cornerRadiusMetadata.DefaultValue = std::any(UI::CornerRadius{});
        cornerRadiusMetadata.Flags = PropertyFlags::AffectsRender;

        RegisterProperty<UI::FrameworkElement, UI::CornerRadius>(
            "CornerRadius",
            &UI::FrameworkElement::GetCornerRadius,
            &UI::FrameworkElement::SetCornerRadius,
            cornerRadiusMetadata);

        PropertyMetadata dataContextMetadata;
        dataContextMetadata.DefaultValue = std::any();
        dataContextMetadata.Flags = PropertyFlags::Inherits;

        RegisterProperty<UI::FrameworkElement, std::any>(
            "DataContext",
            &UI::FrameworkElement::GetDataContext,
            static_cast<void (UI::FrameworkElement::*)(const std::any&)>(&UI::FrameworkElement::SetDataContext),
            dataContextMetadata);
    }
}
