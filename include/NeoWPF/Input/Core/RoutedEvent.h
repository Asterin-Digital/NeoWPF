#pragma once

#include <string>
#include <memory>

namespace NeoWPF::UI
{
    class UIElement;
}

namespace NeoWPF::Input::Core
{
    enum class RoutingStrategy
    {
        Tunnel,
        Bubble,
        Direct
    };

    class RoutedEventArgs
    {
    public:
        RoutedEventArgs(RoutingStrategy strategy);
        virtual ~RoutedEventArgs() = default;

        RoutingStrategy GetRoutingStrategy() const { return m_routingStrategy; }

        NeoWPF::UI::UIElement* GetSource() const { return m_source; }
        void SetSource(NeoWPF::UI::UIElement* source) { m_source = source; }

        NeoWPF::UI::UIElement* GetOriginalSource() const { return m_originalSource; }
        void SetOriginalSource(NeoWPF::UI::UIElement* originalSource) { m_originalSource = originalSource; }

        bool IsHandled() const { return m_handled; }
        void SetHandled(bool handled) { m_handled = handled; }

    protected:
        RoutingStrategy m_routingStrategy;
        NeoWPF::UI::UIElement* m_source = nullptr;
        NeoWPF::UI::UIElement* m_originalSource = nullptr;
        bool m_handled = false;
    };

    struct RoutedEvent
    {
        std::string Name;
        RoutingStrategy Strategy;
        std::size_t EventTypeId;

        RoutedEvent(const std::string& name, RoutingStrategy strategy, std::size_t typeId)
            : Name(name), Strategy(strategy), EventTypeId(typeId)
        {
        }

        bool operator==(const RoutedEvent& other) const
        {
            return EventTypeId == other.EventTypeId;
        }
    };
}
