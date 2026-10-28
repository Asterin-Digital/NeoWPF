/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
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
    class RoutedEventArgs : public virtual InputEventArgs
    {
    public:
        RoutedEventArgs(RoutingStrategy strategy);
        virtual ~RoutedEventArgs() = default;

        RoutingStrategy GetRoutingStrategy() const { return m_routingStrategy; }

        NeoWPF::UI::UIElement* GetSource() const { return m_source; }
        void SetSource(NeoWPF::UI::UIElement* source) { m_source = source; }

        NeoWPF::UI::UIElement* GetOriginalSource() const { return m_originalSource; }
        void SetOriginalSource(NeoWPF::UI::UIElement* originalSource) { m_originalSource = originalSource; }

        const RoutedEvent* GetRoutedEvent() const { return m_routedEvent; }
        void SetRoutedEvent(const RoutedEvent* routedEvent);

    protected:
        RoutingStrategy m_routingStrategy;
        NeoWPF::UI::UIElement* m_source = nullptr;
        NeoWPF::UI::UIElement* m_originalSource = nullptr;
        const RoutedEvent* m_routedEvent = nullptr;
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

    /*
        @summary
        Adds routing state to a pointer packet.

        @internal
    */
    class RoutedPointerEventArgs : public PointerEventArgs, public RoutedEventArgs
    {
    public:
        /*
            @summary
            Creates routed arguments from a pointer packet.

            @internal
        */
        RoutedPointerEventArgs(const RoutedEvent& routedEvent,
                               const PointerEventArgs& pointerEvent)
                        : InputEventArgs(pointerEvent),
                            PointerEventArgs(pointerEvent),
                            RoutedEventArgs(routedEvent.Strategy)
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

    /*
        @summary
        Returns the routed pointer event definition.

        @internal
    */
    const RoutedEvent& PointerEvent();
    /*
        @summary
        Returns the preview pointer-pressed event definition.
    */
    const RoutedEvent& PreviewPointerPressedEvent();
    /*
        @summary
        Returns the pointer-pressed event definition.
    */
    const RoutedEvent& PointerPressedEvent();
    /*
        @summary
        Returns the pointer-moved event definition.
    */
    const RoutedEvent& PointerMovedEvent();
    /*
        @summary
        Returns the pointer-released event definition.
    */
    const RoutedEvent& PointerReleasedEvent();
    /*
        @summary
        Returns the pointer-entered event definition.
    */
    const RoutedEvent& PointerEnteredEvent();
    /*
        @summary
        Returns the pointer-exited event definition.
    */
    const RoutedEvent& PointerExitedEvent();
    /*
        @summary
        Returns the pointer-canceled event definition.
    */
    const RoutedEvent& PointerCanceledEvent();
    /*
        @summary
        Returns the pointer-wheel event definition.
    */
    const RoutedEvent& PointerWheelEvent();
    const RoutedEvent& PreviewKeyDownEvent();
    const RoutedEvent& PreviewKeyUpEvent();
    const RoutedEvent& KeyDownEvent();
    const RoutedEvent& KeyUpEvent();
}

