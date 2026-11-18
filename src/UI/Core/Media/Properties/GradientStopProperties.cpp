#include <NeoWPF/UI/Core/Media/GradientStop.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI {

const Properties::DependencyProperty& GradientStop::ColorProperty()
{
    static const auto& property = Properties::DependencyProperty::Register<
        GradientStop, Color>(
            "Color", Properties::PropertyMetadata{Color{0, 0, 0, 0}});
    return property;
}

const Properties::DependencyProperty& GradientStop::OffsetProperty()
{
    static const auto& property = []() -> const Properties::DependencyProperty& {
        Properties::PropertyMetadata metadata;
        metadata.DefaultValue = 0.0;
        metadata.Converter = Properties::GetDoubleConverter();
        return Properties::DependencyProperty::Register<GradientStop, double>(
            "Offset", metadata);
    }();
    return property;
}

}

namespace NeoWPF::UI::Properties {

void RegisterGradientStopProperties()
{
    (void)UI::GradientStop::ColorProperty();
    (void)UI::GradientStop::OffsetProperty();
}

}
