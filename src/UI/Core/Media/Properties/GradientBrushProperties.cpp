#include <NeoWPF/UI/Core/Media/GradientBrush.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI {

const Properties::DependencyProperty& GradientBrush::GradientStopsProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<
            GradientBrush, std::shared_ptr<GradientStopCollection>>(
                "GradientStops",
                Properties::PropertyMetadata{std::shared_ptr<GradientStopCollection>{}});
    return property;
}

const Properties::DependencyProperty& GradientBrush::MappingModeProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<GradientBrush, BrushMappingMode>(
            "MappingMode",
            Properties::PropertyMetadata{BrushMappingMode::RelativeToBoundingBox});
    return property;
}

const Properties::DependencyProperty& GradientBrush::SpreadMethodProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<GradientBrush, GradientSpreadMethod>(
            "SpreadMethod",
            Properties::PropertyMetadata{GradientSpreadMethod::Pad});
    return property;
}

const Properties::DependencyProperty& GradientBrush::ColorInterpolationModeProperty()
{
    static const auto& property =
        Properties::DependencyProperty::Register<GradientBrush, ColorInterpolationMode>(
            "ColorInterpolationMode",
            Properties::PropertyMetadata{ColorInterpolationMode::SRgb});
    return property;
}

}

namespace NeoWPF::UI::Properties {

void RegisterGradientBrushProperties()
{
    (void)UI::GradientBrush::GradientStopsProperty();
    (void)UI::GradientBrush::MappingModeProperty();
    (void)UI::GradientBrush::SpreadMethodProperty();
    (void)UI::GradientBrush::ColorInterpolationModeProperty();
}

}
