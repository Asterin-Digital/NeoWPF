#include <NeoWPF/UI/Core/Media/Brush.h>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::UI {

NeoWPF::Rendering::Paint ResolveBrushPaint(
    const std::shared_ptr<Brush>& brush,
    NeoWPF::Rendering::CommandList& commandList,
    double boundsX,
    double boundsY,
    double boundsWidth,
    double boundsHeight)
{
    return brush
        ? brush->ResolvePaint(commandList, boundsX, boundsY, boundsWidth, boundsHeight)
        : NeoWPF::Rendering::Paint::Solid(0);
}

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
    return m_changed.Subscribe(std::move(handler));
}

void Brush::RemoveChangedHandler(std::size_t handlerId)
{
    m_changed.Unsubscribe(handlerId);
}

void Brush::NotifyChanged()
{
    m_changed.Publish();
}

void Brush::RefreshTransformSubscriptions()
{
    m_transformSubscription.Subscribe(GetTransform(), [this] {
        NotifyChanged();
    });
    m_relativeTransformSubscription.Subscribe(GetRelativeTransform(), [this] {
        NotifyChanged();
    });
}

void Brush::OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                        const Properties::PropertyMetadata& metadata,
                                        const std::any& oldValue,
                                        const std::any& newValue,
                                        Properties::PropertyValueSource source)
{
    Properties::DependencyObject::OnDependencyPropertyChanged(
        property, metadata, oldValue, newValue, source);

    if (property.GetName() == "Transform" ||
        property.GetName() == "RelativeTransform") {
        RefreshTransformSubscriptions();
    }

    NotifyChanged();
}

BrushSubscription::BrushSubscription(BrushSubscription&& other) noexcept
    : m_brush(std::move(other.m_brush)),
      m_handlerId(std::exchange(other.m_handlerId, 0))
{
}

BrushSubscription& BrushSubscription::operator=(BrushSubscription&& other) noexcept
{
    if (this != &other) {
        Reset();
        m_brush = std::move(other.m_brush);
        m_handlerId = std::exchange(other.m_handlerId, 0);
    }
    return *this;
}

BrushSubscription::~BrushSubscription()
{
    Reset();
}

void BrushSubscription::Subscribe(
    const std::shared_ptr<Brush>& brush,
    Brush::ChangedHandler handler)
{
    Reset();
    if (!brush || !handler) {
        return;
    }

    m_handlerId = brush->AddChangedHandler(std::move(handler));
    if (m_handlerId != 0) {
        m_brush = brush;
    }
}

void BrushSubscription::Reset() noexcept
{
    if (const auto brush = m_brush.lock(); brush && m_handlerId != 0) {
        brush->RemoveChangedHandler(m_handlerId);
    }
    m_brush.reset();
    m_handlerId = 0;
}

bool BrushSubscription::IsSubscribed() const noexcept
{
    return m_handlerId != 0 && !m_brush.expired();
}

}