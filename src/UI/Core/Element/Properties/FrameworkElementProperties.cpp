#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Layout/Responsive/ResponsiveRule.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>

namespace {
    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementNameMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(std::string{});
        metadata.Converter = NeoWPF::UI::Properties::GetStringConverter();
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementResponsiveRulesMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue =
            std::vector<std::shared_ptr<NeoWPF::UI::ResponsiveRule>>{};
        return metadata;
    }
    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementWidthMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(0.0);
        metadata.Converter = NeoWPF::UI::Properties::GetDoubleConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementHeightMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(0.0);
        metadata.Converter = NeoWPF::UI::Properties::GetDoubleConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementMinMaxMetadata(double defaultValue) {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(defaultValue);
        metadata.Converter = NeoWPF::UI::Properties::GetDoubleConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementMinWidthMetadata() {
        return MakeFrameworkElementMinMaxMetadata(0.0);
    }
    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementMaxWidthMetadata() {
        return MakeFrameworkElementMinMaxMetadata(std::numeric_limits<double>::infinity());
    }
    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementMinHeightMetadata() {
        return MakeFrameworkElementMinMaxMetadata(0.0);
    }
    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementMaxHeightMetadata() {
        return MakeFrameworkElementMinMaxMetadata(std::numeric_limits<double>::infinity());
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementWidthModeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::LayoutSizeMode::Auto);
        metadata.Converter = NeoWPF::UI::Properties::GetLayoutSizeModeConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementHeightModeMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::LayoutSizeMode::Auto);
        metadata.Converter = NeoWPF::UI::Properties::GetLayoutSizeModeConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementClipToBoundsMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(false);
        metadata.Converter = NeoWPF::UI::Properties::GetBoolConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementHorizontalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::HorizontalAlignment::Stretch);
        metadata.Converter = NeoWPF::UI::Properties::GetHorizontalAlignmentConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementVerticalAlignmentMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::VerticalAlignment::Stretch);
        metadata.Converter = NeoWPF::UI::Properties::GetVerticalAlignmentConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementMarginMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Thickness{});
        metadata.Converter = NeoWPF::UI::Properties::GetThicknessConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementSpacingMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::Spacing{});
        metadata.Converter = NeoWPF::UI::Properties::GetSpacingConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsMeasure | NeoWPF::UI::Properties::PropertyFlags::AffectsArrange | NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementCornerRadiusMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any(NeoWPF::UI::CornerRadius{});
        metadata.Converter = NeoWPF::UI::Properties::GetCornerRadiusConverter();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsRender;
        return metadata;
    }

    NeoWPF::UI::Properties::PropertyMetadata MakeFrameworkElementDataContextMetadata() {
        NeoWPF::UI::Properties::PropertyMetadata metadata;
        metadata.DefaultValue = std::any();
        metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::Inherits;
        return metadata;
    }
}

namespace NeoWPF::UI {

const Properties::DependencyProperty& FrameworkElement::ResponsiveRulesProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<
            FrameworkElement,
            std::vector<std::shared_ptr<ResponsiveRule>>>(
                "ResponsiveRules",
                [] {
                    Properties::PropertyMetadata metadata;
                    metadata.DefaultValue =
                        std::vector<std::shared_ptr<ResponsiveRule>>{};
                    return metadata;
                }());
    return property;
}

}

namespace NeoWPF::UI::Properties {
    void RegisterFrameworkElementProperties() {
        RegisterProperty<UI::FrameworkElement, std::string>("Name",
            &UI::FrameworkElement::GetName,
            &UI::FrameworkElement::SetName,
            MakeFrameworkElementNameMetadata());
        RegisterProperty<UI::FrameworkElement, double>("Width",
            &UI::FrameworkElement::GetWidth,
            &UI::FrameworkElement::SetWidth,
            MakeFrameworkElementWidthMetadata());
        RegisterProperty<UI::FrameworkElement, double>("Height",
            &UI::FrameworkElement::GetHeight,
            &UI::FrameworkElement::SetHeight,
            MakeFrameworkElementHeightMetadata());
        RegisterProperty<UI::FrameworkElement, double>("MinWidth",
            &UI::FrameworkElement::GetMinWidth,
            &UI::FrameworkElement::SetMinWidth,
            MakeFrameworkElementMinMaxMetadata(0.0));
        RegisterProperty<UI::FrameworkElement, double>("MaxWidth",
            &UI::FrameworkElement::GetMaxWidth,
            &UI::FrameworkElement::SetMaxWidth,
            MakeFrameworkElementMinMaxMetadata(std::numeric_limits<double>::infinity()));
        RegisterProperty<UI::FrameworkElement, double>("MinHeight",
            &UI::FrameworkElement::GetMinHeight,
            &UI::FrameworkElement::SetMinHeight,
            MakeFrameworkElementMinMaxMetadata(0.0));
        RegisterProperty<UI::FrameworkElement, double>("MaxHeight",
            &UI::FrameworkElement::GetMaxHeight,
            &UI::FrameworkElement::SetMaxHeight,
            MakeFrameworkElementMinMaxMetadata(std::numeric_limits<double>::infinity()));
        RegisterProperty<UI::FrameworkElement, UI::LayoutSizeMode>("WidthMode",
            &UI::FrameworkElement::GetWidthMode,
            &UI::FrameworkElement::SetWidthMode,
            MakeFrameworkElementWidthModeMetadata());
        RegisterProperty<UI::FrameworkElement, UI::LayoutSizeMode>("HeightMode",
            &UI::FrameworkElement::GetHeightMode,
            &UI::FrameworkElement::SetHeightMode,
            MakeFrameworkElementHeightModeMetadata());
        RegisterProperty<UI::FrameworkElement, bool>("ClipToBounds",
            &UI::FrameworkElement::IsClipToBounds,
            &UI::FrameworkElement::SetClipToBounds,
            MakeFrameworkElementClipToBoundsMetadata());
        RegisterProperty<UI::FrameworkElement, UI::HorizontalAlignment>("HorizontalAlignment",
            &UI::FrameworkElement::GetHorizontalAlignment,
            &UI::FrameworkElement::SetHorizontalAlignment,
            MakeFrameworkElementHorizontalAlignmentMetadata());
        RegisterProperty<UI::FrameworkElement, UI::VerticalAlignment>("VerticalAlignment",
            &UI::FrameworkElement::GetVerticalAlignment,
            &UI::FrameworkElement::SetVerticalAlignment,
            MakeFrameworkElementVerticalAlignmentMetadata());
        RegisterProperty<UI::FrameworkElement, UI::Thickness>("Margin",
            &UI::FrameworkElement::GetMargin,
            static_cast<void (UI::FrameworkElement::*)(UI::Thickness)>(&UI::FrameworkElement::SetMargin),
            MakeFrameworkElementMarginMetadata());
        RegisterProperty<UI::FrameworkElement, UI::Spacing>("Spacing",
            &UI::FrameworkElement::GetSpacing,
            static_cast<void (UI::FrameworkElement::*)(UI::Spacing)>(&UI::FrameworkElement::SetSpacing),
            MakeFrameworkElementSpacingMetadata());
        RegisterProperty<UI::FrameworkElement, UI::CornerRadius>("CornerRadius",
            &UI::FrameworkElement::GetCornerRadius,
            &UI::FrameworkElement::SetCornerRadius,
            MakeFrameworkElementCornerRadiusMetadata());
        RegisterProperty<UI::FrameworkElement, std::any>("DataContext",
            &UI::FrameworkElement::GetDataContext,
            static_cast<void (UI::FrameworkElement::*)(const std::any&)>(&UI::FrameworkElement::SetDataContext),
            MakeFrameworkElementDataContextMetadata());
        NeoWPF::UI::Markup::Xaml::RegisterXamlType<UI::FrameworkElement>(
            "FrameworkElement", "", "NeoWPF::UI::FrameworkElement", "NeoWPF/UI/Core/Element/FrameworkElement.h");
        NeoWPF::UI::Markup::Xaml::RegisterXamlCollectionProperty<
            UI::FrameworkElement, UI::ResponsiveRule>(
                UI::FrameworkElement::ResponsiveRulesProperty(),
                &UI::FrameworkElement::AddResponsiveRule,
                "AddResponsiveRule");
    }
}

DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             Width,
                             double,
                             &NeoWPF::UI::FrameworkElement::GetWidth,
                             &NeoWPF::UI::FrameworkElement::SetWidth,
                             MakeFrameworkElementWidthMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             Name,
                             std::string,
                             &NeoWPF::UI::FrameworkElement::GetName,
                             &NeoWPF::UI::FrameworkElement::SetName,
                             MakeFrameworkElementNameMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             Height,
                             double,
                             &NeoWPF::UI::FrameworkElement::GetHeight,
                             &NeoWPF::UI::FrameworkElement::SetHeight,
                             MakeFrameworkElementHeightMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             MinWidth,
                             double,
                             &NeoWPF::UI::FrameworkElement::GetMinWidth,
                             &NeoWPF::UI::FrameworkElement::SetMinWidth,
                             MakeFrameworkElementMinWidthMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             MaxWidth,
                             double,
                             &NeoWPF::UI::FrameworkElement::GetMaxWidth,
                             &NeoWPF::UI::FrameworkElement::SetMaxWidth,
                             MakeFrameworkElementMaxWidthMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             MinHeight,
                             double,
                             &NeoWPF::UI::FrameworkElement::GetMinHeight,
                             &NeoWPF::UI::FrameworkElement::SetMinHeight,
                             MakeFrameworkElementMinHeightMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             MaxHeight,
                             double,
                             &NeoWPF::UI::FrameworkElement::GetMaxHeight,
                             &NeoWPF::UI::FrameworkElement::SetMaxHeight,
                             MakeFrameworkElementMaxHeightMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             WidthMode,
                             NeoWPF::UI::LayoutSizeMode,
                             &NeoWPF::UI::FrameworkElement::GetWidthMode,
                             &NeoWPF::UI::FrameworkElement::SetWidthMode,
                             MakeFrameworkElementWidthModeMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             HeightMode,
                             NeoWPF::UI::LayoutSizeMode,
                             &NeoWPF::UI::FrameworkElement::GetHeightMode,
                             &NeoWPF::UI::FrameworkElement::SetHeightMode,
                             MakeFrameworkElementHeightModeMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             ClipToBounds,
                             bool,
                             &NeoWPF::UI::FrameworkElement::IsClipToBounds,
                             &NeoWPF::UI::FrameworkElement::SetClipToBounds,
                             MakeFrameworkElementClipToBoundsMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             HorizontalAlignment,
                             NeoWPF::UI::HorizontalAlignment,
                             &NeoWPF::UI::FrameworkElement::GetHorizontalAlignment,
                             &NeoWPF::UI::FrameworkElement::SetHorizontalAlignment,
                             MakeFrameworkElementHorizontalAlignmentMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             VerticalAlignment,
                             NeoWPF::UI::VerticalAlignment,
                             &NeoWPF::UI::FrameworkElement::GetVerticalAlignment,
                             &NeoWPF::UI::FrameworkElement::SetVerticalAlignment,
                             MakeFrameworkElementVerticalAlignmentMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             Margin,
                             NeoWPF::UI::Thickness,
                             &NeoWPF::UI::FrameworkElement::GetMargin,
                             static_cast<void (NeoWPF::UI::FrameworkElement::*)(NeoWPF::UI::Thickness)>(&NeoWPF::UI::FrameworkElement::SetMargin),
                             MakeFrameworkElementMarginMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             Spacing,
                             NeoWPF::UI::Spacing,
                             &NeoWPF::UI::FrameworkElement::GetSpacing,
                             static_cast<void (NeoWPF::UI::FrameworkElement::*)(NeoWPF::UI::Spacing)>(&NeoWPF::UI::FrameworkElement::SetSpacing),
                             MakeFrameworkElementSpacingMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             CornerRadius,
                             NeoWPF::UI::CornerRadius,
                             &NeoWPF::UI::FrameworkElement::GetCornerRadius,
                             &NeoWPF::UI::FrameworkElement::SetCornerRadius,
                             MakeFrameworkElementCornerRadiusMetadata)
DEFINE_STATIC_PROPERTY_HANDLE(NeoWPF::UI::FrameworkElement,
                             DataContext,
                             std::any,
                             &NeoWPF::UI::FrameworkElement::GetDataContext,
                             static_cast<void (NeoWPF::UI::FrameworkElement::*)(const std::any&)>(&NeoWPF::UI::FrameworkElement::SetDataContext),
                             MakeFrameworkElementDataContextMetadata)
