#include <NeoWPF/UI/Core/Media/GradientStop.h>

namespace NeoWPF::UI {

GradientStop::GradientStop(Color color, double offset)
{
    SetColor(color);
    SetOffset(offset);
}

Color GradientStop::GetColor() const
{
    return GetValue<Color>(ColorProperty());
}

void GradientStop::SetColor(Color color)
{
    SetValue(ColorProperty(), color);
}

double GradientStop::GetOffset() const
{
    return GetValue<double>(OffsetProperty());
}

void GradientStop::SetOffset(double offset)
{
    SetValue(OffsetProperty(), offset);
}

std::size_t GradientStop::AddChangedHandler(std::function<void()> handler)
{
    return m_changed.Subscribe(std::move(handler));
}

void GradientStop::RemoveChangedHandler(std::size_t handlerId)
{
    m_changed.Unsubscribe(handlerId);
}

void GradientStop::OnDependencyPropertyChanged(
    const Properties::DependencyProperty& property,
    const Properties::PropertyMetadata& metadata,
    const std::any& oldValue,
    const std::any& newValue,
    Properties::PropertyValueSource source)
{
    Properties::DependencyObject::OnDependencyPropertyChanged(
        property, metadata, oldValue, newValue, source);
    m_changed.Publish();
}

}
