#pragma once

#include <functional>
#include <memory>

namespace NeoWPF::UI
{
    class UIElement;
}

namespace NeoWPF::Input::Core
{
    class RoutedEventArgs;

    template<typename TArgs>
    using EventHandler = std::function<void(::NeoWPF::UI::UIElement*, TArgs*)>;

    using MouseEventHandler = EventHandler<class MouseEventArgs>;
    using KeyboardEventHandler = EventHandler<class KeyboardEventArgs>;
    using RoutedEventHandler = EventHandler<class RoutedEventArgs>;
}
