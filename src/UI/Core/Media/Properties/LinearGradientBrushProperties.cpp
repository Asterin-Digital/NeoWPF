#include <NeoWPF/UI/Core/Media/GradientBrush.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI {

const Properties::DependencyProperty& LinearGradientBrush::StartPointProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        LinearGradientBrush, GradientPoint>(
            "StartPoint", Properties::PropertyMetadata{GradientPoint{0.0, 0.0}});
    return property;
}

const Properties::DependencyProperty& LinearGradientBrush::EndPointProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        LinearGradientBrush, GradientPoint>(
            "EndPoint", Properties::PropertyMetadata{GradientPoint{1.0, 1.0}});
    return property;
}

}

namespace NeoWPF::UI::Properties {

void RegisterLinearGradientBrushProperties()
{
    (void)UI::LinearGradientBrush::StartPointProperty();
    (void)UI::LinearGradientBrush::EndPointProperty();
}

}
