/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <memory>
#include <NeoWPF/Input/Core/Events.h>

namespace NeoWPF::UI
{
    class UIElement;
}

namespace NeoWPF::Input::Core
{
    struct RoutedEvent;

    /*
        @summary
        Defines how a routed event travels through the visual tree.
    */
    enum class RoutingStrategy
    {
        Tunnel,
        Bubble,
        Direct
    };

    /*
        @summary
        Carries routing state for an input event.
    */
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

        const RoutedEvent* GetRoutedEvent() const { return m_routedEvent; }
        void SetRoutedEvent(const RoutedEvent* routedEvent) { m_routedEvent = routedEvent; }

    protected:
        RoutingStrategy m_routingStrategy;
        NeoWPF::UI::UIElement* m_source = nullptr;
        NeoWPF::UI::UIElement* m_originalSource = nullptr;
        const RoutedEvent* m_routedEvent = nullptr;
        bool m_handled = false;
    };

    /*
        @summary
        Describes a routed event type.
    */
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

    class RoutedMouseEventArgs : public MouseEventArgs, public RoutedEventArgs
    {
    public:
        RoutedMouseEventArgs(const RoutedEvent& routedEvent, int x, int y, std::uint32_t buttonState, int wheelDelta = 0)
            : MouseEventArgs(x, y, buttonState, wheelDelta), RoutedEventArgs(routedEvent.Strategy)
        {
            SetRoutedEvent(&routedEvent);
        }
    };

    class RoutedKeyboardEventArgs : public KeyboardEventArgs, public RoutedEventArgs
    {
    public:
        RoutedKeyboardEventArgs(const RoutedEvent& routedEvent, int keyCode, bool isKeyDown, wchar_t charCode = 0, int repeatCount = 1)
            : KeyboardEventArgs(keyCode, isKeyDown, charCode, repeatCount), RoutedEventArgs(routedEvent.Strategy)
        {
            SetRoutedEvent(&routedEvent);
        }
    };

    const RoutedEvent& PreviewMouseDownEvent();
    const RoutedEvent& PreviewMouseUpEvent();
    const RoutedEvent& PreviewMouseMoveEvent();
    const RoutedEvent& PreviewMouseWheelEvent();
    const RoutedEvent& MouseDownEvent();
    const RoutedEvent& MouseUpEvent();
    const RoutedEvent& MouseMoveEvent();
    const RoutedEvent& MouseWheelEvent();
    const RoutedEvent& MouseEnterEvent();
    const RoutedEvent& MouseLeaveEvent();
    const RoutedEvent& PreviewKeyDownEvent();
    const RoutedEvent& PreviewKeyUpEvent();
    const RoutedEvent& KeyDownEvent();
    const RoutedEvent& KeyUpEvent();
}

