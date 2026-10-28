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

        const Core::RoutedEvent* routedEvent = nullptr;
        switch (eventArgs.GetPhase()) {
        case Core::PointerPhase::Enter:
            routedEvent = &Core::PointerEnteredEvent();
            break;
        case Core::PointerPhase::Move:
            routedEvent = &Core::PointerMovedEvent();
            break;
        case Core::PointerPhase::Down:
            routedEvent = &Core::PreviewPointerPressedEvent();
            break;
        case Core::PointerPhase::Up:
            routedEvent = &Core::PointerReleasedEvent();
            break;
        case Core::PointerPhase::Cancel:
            routedEvent = &Core::PointerCanceledEvent();
            break;
        case Core::PointerPhase::Leave:
            routedEvent = &Core::PointerExitedEvent();
            break;
        case Core::PointerPhase::Wheel:
            routedEvent = &Core::PointerWheelEvent();
            break;
        }
        if (!routedEvent) {
            return;
        }

        Core::RoutedPointerEventArgs args(*routedEvent, eventArgs);
        args.SetSource(targetElement);
        args.SetOriginalSource(targetElement);
        targetElement->RaiseEvent(&args);
        if (eventArgs.GetPhase() == Core::PointerPhase::Down && !args.IsHandled()) {
            args.SetRoutedEvent(&Core::PointerPressedEvent());
            targetElement->RaiseEvent(&args);
        }
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

    const RoutedEvent& PreviewPointerPressedEvent()
    {
        static const RoutedEvent event("PreviewPointerPressed", RoutingStrategy::Tunnel,
            reinterpret_cast<std::size_t>(&PreviewPointerPressedEvent));
        return event;
    }

    const RoutedEvent& PointerPressedEvent()
    {
        static const RoutedEvent event("PointerPressed", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&PointerPressedEvent));
        return event;
    }

    const RoutedEvent& PointerMovedEvent()
    {
        static const RoutedEvent event("PointerMoved", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&PointerMovedEvent));
        return event;
    }

    const RoutedEvent& PointerReleasedEvent()
    {
        static const RoutedEvent event("PointerReleased", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&PointerReleasedEvent));
        return event;
    }

    const RoutedEvent& PointerEnteredEvent()
    {
        static const RoutedEvent event("PointerEntered", RoutingStrategy::Direct,
            reinterpret_cast<std::size_t>(&PointerEnteredEvent));
        return event;
    }

    const RoutedEvent& PointerExitedEvent()
    {
        static const RoutedEvent event("PointerExited", RoutingStrategy::Direct,
            reinterpret_cast<std::size_t>(&PointerExitedEvent));
        return event;
    }

    const RoutedEvent& PointerCanceledEvent()
    {
        static const RoutedEvent event("PointerCanceled", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&PointerCanceledEvent));
        return event;
    }

    const RoutedEvent& PointerWheelEvent()
    {
        static const RoutedEvent event("PointerWheel", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&PointerWheelEvent));
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
