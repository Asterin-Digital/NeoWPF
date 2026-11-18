#include <NeoWPF/UI/Core/Media/GradientBrush.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI {

const Properties::DependencyProperty& RadialGradientBrush::CenterProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        RadialGradientBrush, GradientPoint>(
            "Center", Properties::PropertyMetadata{GradientPoint{0.5, 0.5}});
    return property;
}

const Properties::DependencyProperty& RadialGradientBrush::GradientOriginProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        RadialGradientBrush, GradientPoint>(
            "GradientOrigin", Properties::PropertyMetadata{GradientPoint{0.5, 0.5}});
    return property;
}

const Properties::DependencyProperty& RadialGradientBrush::RadiusXProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        RadialGradientBrush, double>(
            "RadiusX", Properties::PropertyMetadata{0.5});
    return property;
}

const Properties::DependencyProperty& RadialGradientBrush::RadiusYProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        RadialGradientBrush, double>(
            "RadiusY", Properties::PropertyMetadata{0.5});
    return property;
}

}

namespace NeoWPF::UI::Properties {

void RegisterRadialGradientBrushProperties()
{
    (void)UI::RadialGradientBrush::CenterProperty();
    (void)UI::RadialGradientBrush::GradientOriginProperty();
    (void)UI::RadialGradientBrush::RadiusXProperty();
    (void)UI::RadialGradientBrush::RadiusYProperty();
}

}
