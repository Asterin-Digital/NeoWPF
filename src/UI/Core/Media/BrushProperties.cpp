#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI {

const Properties::DependencyProperty& Brush::OpacityProperty()
{
    static const auto& property = []() -> const Properties::DependencyProperty& {
        Properties::PropertyMetadata metadata;
        metadata.DefaultValue = 1.0;
        metadata.Converter = Properties::GetDoubleConverter();
        metadata.Flags = Properties::PropertyFlags::AffectsRender;
        return Properties::DependencyProperty::Register<Brush, double>("Opacity", metadata);
    }();
    return property;
}

const Properties::DependencyProperty& Brush::TransformProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        Brush, std::shared_ptr<Core::Transform>>(
            "Transform", Properties::PropertyMetadata{std::shared_ptr<Core::Transform>{}});
    return property;
}

const Properties::DependencyProperty& Brush::RelativeTransformProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        Brush, std::shared_ptr<Core::Transform>>(
            "RelativeTransform", Properties::PropertyMetadata{std::shared_ptr<Core::Transform>{}});
    return property;
}

const Properties::DependencyProperty& SolidColorBrush::ColorProperty()
{
    static const auto& property = []() -> const Properties::DependencyProperty& {
        Properties::PropertyMetadata metadata;
        metadata.DefaultValue = Colors::Transparent;
        metadata.Converter = Properties::GetColorConverter();
        metadata.Flags = Properties::PropertyFlags::AffectsRender;
        return Properties::DependencyProperty::Register<SolidColorBrush, Color>("Color", metadata);
    }();
    return property;
}

}

namespace NeoWPF::UI::Properties {

void RegisterBrushProperties()
{
    (void)UI::Brush::OpacityProperty();
    (void)UI::Brush::TransformProperty();
    (void)UI::Brush::RelativeTransformProperty();
}

void RegisterSolidColorBrushProperties()
{
    RegisterBrushProperties();
    (void)UI::SolidColorBrush::ColorProperty();
    UI::Markup::Xaml::RegisterXamlType<UI::SolidColorBrush>(
        "SolidColorBrush", "", "NeoWPF::UI::SolidColorBrush",
        "NeoWPF/UI/Core/Media/SolidColorBrush.h");
}

}