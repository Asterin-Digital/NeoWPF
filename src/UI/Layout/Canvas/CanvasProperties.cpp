#include <NeoWPF/UI/Layout/Canvas/Canvas.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

#include <any>
#include <cmath>
#include <limits>

namespace {

NeoWPF::UI::Properties::PropertyMetadata MakeCanvasCoordinateMetadata()
{
    NeoWPF::UI::Properties::PropertyMetadata metadata;
    metadata.DefaultValue = std::any(std::numeric_limits<double>::quiet_NaN());
    metadata.Converter = NeoWPF::UI::Properties::GetDoubleConverter();
    metadata.Flags = NeoWPF::UI::Properties::PropertyFlags::AffectsArrange;
    return metadata;
}

template<typename Getter>
double GetCanvasCoordinate(const NeoWPF::UI::Visual* element,
                           const NeoWPF::UI::Properties::DependencyProperty& property,
                           Getter getter) noexcept
{
    if (!element) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    if (const auto* object = dynamic_cast<const NeoWPF::UI::Properties::DependencyObject*>(element)) {
        try {
            return getter(*object, property);
        } catch (const std::bad_any_cast&) {
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

void SetCanvasCoordinate(NeoWPF::UI::Visual* element,
                         const NeoWPF::UI::Properties::DependencyProperty& property,
                         double value) noexcept
{
    if (!element) {
        return;
    }
    if (auto* object = dynamic_cast<NeoWPF::UI::Properties::DependencyObject*>(element)) {
        if (std::isnan(value)) {
            object->ClearValue(property);
        } else {
            object->SetValue(property, value);
        }
    }
}

}

namespace NeoWPF::UI::Properties {

void RegisterCanvasProperties()
{
    RegisterAttachedProperty<::NeoWPF::UI::Canvas, ::NeoWPF::UI::Visual, double>("Left", MakeCanvasCoordinateMetadata());
    RegisterAttachedProperty<::NeoWPF::UI::Canvas, ::NeoWPF::UI::Visual, double>("Top", MakeCanvasCoordinateMetadata());
    RegisterAttachedProperty<::NeoWPF::UI::Canvas, ::NeoWPF::UI::Visual, double>("Right", MakeCanvasCoordinateMetadata());
    RegisterAttachedProperty<::NeoWPF::UI::Canvas, ::NeoWPF::UI::Visual, double>("Bottom", MakeCanvasCoordinateMetadata());

    ::NeoWPF::UI::Markup::Xaml::RegisterXamlType<::NeoWPF::UI::Canvas>(
        "Canvas", "Children", "NeoWPF::UI::Canvas", "NeoWPF/UI/Layout/Canvas/Canvas.h");
}

}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Canvas::LeftProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::RegisterAttached<NeoWPF::UI::Canvas, NeoWPF::UI::Visual, double>("Left", MakeCanvasCoordinateMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Canvas::TopProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::RegisterAttached<NeoWPF::UI::Canvas, NeoWPF::UI::Visual, double>("Top", MakeCanvasCoordinateMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Canvas::RightProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::RegisterAttached<NeoWPF::UI::Canvas, NeoWPF::UI::Visual, double>("Right", MakeCanvasCoordinateMetadata());
    return property;
}

const NeoWPF::UI::Properties::DependencyProperty& NeoWPF::UI::Canvas::BottomProperty()
{
    static const auto& property = NeoWPF::UI::Properties::DependencyProperty::RegisterAttached<NeoWPF::UI::Canvas, NeoWPF::UI::Visual, double>("Bottom", MakeCanvasCoordinateMetadata());
    return property;
}

double NeoWPF::UI::Canvas::GetLeft(const NeoWPF::UI::Visual* element) noexcept
{
    return GetCanvasCoordinate(element, LeftProperty(), [](const auto& object, const auto& property) {
        return object.template GetValue<double>(property);
    });
}

void NeoWPF::UI::Canvas::SetLeft(NeoWPF::UI::Visual* element, double value) noexcept
{
    SetCanvasCoordinate(element, LeftProperty(), value);
}

double NeoWPF::UI::Canvas::GetTop(const NeoWPF::UI::Visual* element) noexcept
{
    return GetCanvasCoordinate(element, TopProperty(), [](const auto& object, const auto& property) {
        return object.template GetValue<double>(property);
    });
}

void NeoWPF::UI::Canvas::SetTop(NeoWPF::UI::Visual* element, double value) noexcept
{
    SetCanvasCoordinate(element, TopProperty(), value);
}

double NeoWPF::UI::Canvas::GetRight(const NeoWPF::UI::Visual* element) noexcept
{
    return GetCanvasCoordinate(element, RightProperty(), [](const auto& object, const auto& property) {
        return object.template GetValue<double>(property);
    });
}

void NeoWPF::UI::Canvas::SetRight(NeoWPF::UI::Visual* element, double value) noexcept
{
    SetCanvasCoordinate(element, RightProperty(), value);
}

double NeoWPF::UI::Canvas::GetBottom(const NeoWPF::UI::Visual* element) noexcept
{
    return GetCanvasCoordinate(element, BottomProperty(), [](const auto& object, const auto& property) {
        return object.template GetValue<double>(property);
    });
}

void NeoWPF::UI::Canvas::SetBottom(NeoWPF::UI::Visual* element, double value) noexcept
{
    SetCanvasCoordinate(element, BottomProperty(), value);
}