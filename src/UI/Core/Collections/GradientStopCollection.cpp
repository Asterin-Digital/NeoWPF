#include <NeoWPF/UI/Core/Collections/GradientStopCollection.h>

#include <algorithm>

namespace NeoWPF::UI {

GradientStopCollection::~GradientStopCollection()
{
    UnsubscribeFromStops();
}

std::size_t GradientStopCollection::size() const noexcept
{
    return m_stops.size();
}

bool GradientStopCollection::empty() const noexcept
{
    return m_stops.empty();
}

GradientStopCollection::const_iterator GradientStopCollection::begin() const noexcept
{
    return m_stops.begin();
}

GradientStopCollection::const_iterator GradientStopCollection::end() const noexcept
{
    return m_stops.end();
}

void GradientStopCollection::Add(std::shared_ptr<GradientStop> stop)
{
    if (!stop) {
        return;
    }

    m_stopHandlerIds.push_back(stop->AddChangedHandler([this] {
        m_changed.Publish();
    }));
    m_stops.push_back(std::move(stop));
    m_changed.Publish();
}

bool GradientStopCollection::Remove(const std::shared_ptr<GradientStop>& stop)
{
    const auto found = std::find(m_stops.begin(), m_stops.end(), stop);
    if (found == m_stops.end()) {
        return false;
    }

    const auto index = static_cast<std::size_t>(std::distance(m_stops.begin(), found));
    (*found)->RemoveChangedHandler(m_stopHandlerIds[index]);
    m_stops.erase(found);
    m_stopHandlerIds.erase(m_stopHandlerIds.begin() + static_cast<std::ptrdiff_t>(index));
    m_changed.Publish();
    return true;
}

void GradientStopCollection::Clear()
{
    if (m_stops.empty()) {
        return;
    }
    UnsubscribeFromStops();
    m_stops.clear();
    m_stopHandlerIds.clear();
    m_changed.Publish();
}

std::size_t GradientStopCollection::AddChangedHandler(std::function<void()> handler)
{
    return m_changed.Subscribe(std::move(handler));
}

void GradientStopCollection::RemoveChangedHandler(std::size_t handlerId)
{
    m_changed.Unsubscribe(handlerId);
}

void GradientStopCollection::UnsubscribeFromStops() noexcept
{
    for (std::size_t index = 0; index < m_stops.size(); ++index) {
        if (m_stops[index]) {
            m_stops[index]->RemoveChangedHandler(m_stopHandlerIds[index]);
        }
    }
}

}
