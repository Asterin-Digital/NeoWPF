#pragma once

#include <functional>
#include <memory>

namespace NeoWPF::Input::Core
{
    class RoutedEventArgs;
    class UIElement;

    template<typename TArgs>
    using EventHandler = std::function<void(UIElement*, TArgs*)>;

    using MouseEventHandler = EventHandler<class MouseEventArgs>;
    using KeyboardEventHandler = EventHandler<class KeyboardEventArgs>;
    using RoutedEventHandler = EventHandler<class RoutedEventArgs>;
}
