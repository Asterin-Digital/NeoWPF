#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::Input
{
    void InputManager::RouteMouseEvent(NeoWPF::UI::UIElement* targetElement, const Core::RoutedEvent& eventDef,
                                      int x, int y, std::uint32_t buttonState, int wheelDelta)
    {
        if (!targetElement)
        {
            return;
        }

        Core::RoutedMouseEventArgs args(eventDef, x, y, buttonState, wheelDelta);
        args.SetSource(targetElement);
        args.SetOriginalSource(targetElement);
        targetElement->RaiseEvent(&args);
    }

    void InputManager::RouteKeyboardEvent(NeoWPF::UI::UIElement* targetElement, const Core::RoutedEvent& eventDef,
                                         int keyCode, bool isKeyDown, wchar_t charCode, int repeatCount)
    {
        if (!targetElement)
        {
            return;
        }

        Core::RoutedKeyboardEventArgs args(eventDef, keyCode, isKeyDown, charCode, repeatCount);
        args.SetSource(targetElement);
        args.SetOriginalSource(targetElement);
        targetElement->RaiseEvent(&args);
    }
}

namespace NeoWPF::Input::Core
{
    const RoutedEvent& PreviewMouseDownEvent()
    {
        static const RoutedEvent event("PreviewMouseDown", RoutingStrategy::Tunnel,
            reinterpret_cast<std::size_t>(&PreviewMouseDownEvent));
        return event;
    }

    const RoutedEvent& PreviewMouseUpEvent()
    {
        static const RoutedEvent event("PreviewMouseUp", RoutingStrategy::Tunnel,
            reinterpret_cast<std::size_t>(&PreviewMouseUpEvent));
        return event;
    }

    const RoutedEvent& PreviewMouseMoveEvent()
    {
        static const RoutedEvent event("PreviewMouseMove", RoutingStrategy::Tunnel,
            reinterpret_cast<std::size_t>(&PreviewMouseMoveEvent));
        return event;
    }

    const RoutedEvent& PreviewMouseWheelEvent()
    {
        static const RoutedEvent event("PreviewMouseWheel", RoutingStrategy::Tunnel,
            reinterpret_cast<std::size_t>(&PreviewMouseWheelEvent));
        return event;
    }

    const RoutedEvent& MouseDownEvent()
    {
        static const RoutedEvent event("MouseDown", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&MouseDownEvent));
        return event;
    }

    const RoutedEvent& MouseUpEvent()
    {
        static const RoutedEvent event("MouseUp", RoutingStrategy::Bubble,
            reinterpret_cast<std::size_t>(&MouseUpEvent));
        return event;
    }

    const RoutedEvent& MouseMoveEvent()
    {
        static const RoutedEvent event("MouseMove", RoutingStrategy::Direct,
            reinterpret_cast<std::size_t>(&MouseMoveEvent));
        return event;
    }

    const RoutedEvent& MouseWheelEvent()
    {
        static const RoutedEvent event("MouseWheel", RoutingStrategy::Direct,
            reinterpret_cast<std::size_t>(&MouseWheelEvent));
        return event;
    }

    const RoutedEvent& MouseEnterEvent()
    {
        static const RoutedEvent event("MouseEnter", RoutingStrategy::Direct,
            reinterpret_cast<std::size_t>(&MouseEnterEvent));
        return event;
    }

    const RoutedEvent& MouseLeaveEvent()
    {
        static const RoutedEvent event("MouseLeave", RoutingStrategy::Direct,
            reinterpret_cast<std::size_t>(&MouseLeaveEvent));
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
        static const RoutedEvent event("KeyDown", RoutingStrategy::Direct,
            reinterpret_cast<std::size_t>(&KeyDownEvent));
        return event;
    }

    const RoutedEvent& KeyUpEvent()
    {
        static const RoutedEvent event("KeyUp", RoutingStrategy::Direct,
            reinterpret_cast<std::size_t>(&KeyUpEvent));
        return event;
    }

    RoutedEventArgs::RoutedEventArgs(RoutingStrategy strategy)
        : m_routingStrategy(strategy)
    {
    }
}
