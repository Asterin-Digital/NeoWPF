#pragma once

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Platform/Window.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/UI.h>
#include <NeoWPF/Input/Input.h>

namespace NeoWPF {

    using Core::Application;

    using Platform::Window;

    using UI::Panel;

    using UI::StackPanel;

    using UI::Grid;

    using UI::TextBlock;

    using UI::TextBox;

    using UI::Button;

    using Input::InputManager;

    namespace Input {

        using Core::MouseEventArgs;

        using Core::KeyboardEventArgs;

        using Core::RoutedEvent;

        using Core::RoutingStrategy;

        using Core::MouseEventHandler;

        using Core::KeyboardEventHandler;

    }

}