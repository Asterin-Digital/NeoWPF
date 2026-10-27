#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::Input
{
    void InputManager::RoutePointerEvent(NeoWPF::UI::UIElement* targetElement,
                                         const Core::PointerEventArgs& eventArgs)
    {
        if (!targetElement) {
            return;
        }

        Core::RoutedPointerEventArgs args(Core::PointerEvent(), eventArgs);
        args.SetSource(targetElement);
        args.SetOriginalSource(targetElement);
        targetElement->RaiseEvent(&args);
    }

    void InputManager::RouteKeyboardEvent(NeoWPF::UI::UIElement* targetElement,
                                         Core::RoutedKeyboardEventArgs& args,
                                         const Core::RoutedEvent& eventDef)
    {
        if (!targetElement)
        {
            return;
        }

        args.SetRoutedEvent(&eventDef);
        targetElement->RaiseEvent(&args);
    }
}

namespace NeoWPF::Input::Core
{
    const RoutedEvent& PointerEvent()
    {
        static const RoutedEvent event("Pointer", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&PointerEvent));
        return event;
    }

    const RoutedEvent& PreviewKeyDownEvent()
    {
        static const RoutedEvent event("PreviewKeyDown", RoutingStrategy::Tunnel,
            reinterpret_cast<std::size_t>(&PreviewKeyDownEvent));
        return event;
    }

    const RoutedEvent& PreviewKeyUpEvent()
    {
        static const RoutedEvent event("PreviewKeyUp", RoutingStrategy::Tunnel,
            reinterpret_cast<std::size_t>(&PreviewKeyUpEvent));
        return event;
    }

    const RoutedEvent& KeyDownEvent()
    {
        static const RoutedEvent event("KeyDown", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&KeyDownEvent));
        return event;
    }

    const RoutedEvent& KeyUpEvent()
    {
        static const RoutedEvent event("KeyUp", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&KeyUpEvent));
        return event;
    }

    void RoutedEventArgs::SetRoutedEvent(const RoutedEvent* routedEvent)
    {
        m_routedEvent = routedEvent;
        if (routedEvent) {
            m_routingStrategy = routedEvent->Strategy;
        }
    }

    RoutedEventArgs::RoutedEventArgs(RoutingStrategy strategy)
        : m_routingStrategy(strategy)
    {
    }
}
