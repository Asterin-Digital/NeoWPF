/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

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

    using PointerEventHandler = EventHandler<class PointerEventArgs>;
    using KeyboardEventHandler = EventHandler<class KeyboardEventArgs>;
    using RoutedEventHandler = EventHandler<class RoutedEventArgs>;
}
