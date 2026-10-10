#include <NeoWPF/UI/Core/Media/Brush.h>

#include <vector>

namespace NeoWPF::UI {

Brush::~Brush() = default;

double Brush::GetOpacity() const
{
    return GetValue<double>(OpacityProperty());
}

void Brush::SetOpacity(double opacity)
{
    SetValue(OpacityProperty(), opacity);
}

std::shared_ptr<Core::Transform> Brush::GetTransform() const
{
    return GetValue<std::shared_ptr<Core::Transform>>(TransformProperty());
}

void Brush::SetTransform(std::shared_ptr<Core::Transform> transform)
{
    SetValue(TransformProperty(), transform);
}

std::shared_ptr<Core::Transform> Brush::GetRelativeTransform() const
{
    return GetValue<std::shared_ptr<Core::Transform>>(RelativeTransformProperty());
}

void Brush::SetRelativeTransform(std::shared_ptr<Core::Transform> transform)
{
    SetValue(RelativeTransformProperty(), transform);
}

std::size_t Brush::AddChangedHandler(ChangedHandler handler)
{
    if (!handler) {
        return 0;
    }

    const auto handlerId = m_nextChangedHandlerId++;
    m_changedHandlers.emplace(handlerId, std::move(handler));
    return handlerId;
}

void Brush::RemoveChangedHandler(std::size_t handlerId)
{
    m_changedHandlers.erase(handlerId);
}

void Brush::OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                        const Properties::PropertyMetadata& metadata,
                                        const std::any& oldValue,
                                        const std::any& newValue,
                                        Properties::PropertyValueSource source)
{
    Properties::DependencyObject::OnDependencyPropertyChanged(
        property, metadata, oldValue, newValue, source);

    std::vector<ChangedHandler> handlers;
    handlers.reserve(m_changedHandlers.size());
    for (const auto& [handlerId, handler] : m_changedHandlers) {
        (void)handlerId;
        handlers.push_back(handler);
    }

    for (const auto& handler : handlers) {
        handler();
    }
}

}