/*
============================================================

███╗   ██╗███████╗ ██████╗ ██╗    ██╗██████╗ ███████╗
████╗  ██║██╔════╝██╔═══██╗██║    ██║██╔══██╗██╔════╝
██╔██╗ ██║█████╗  ██║   ██║██║ █╗ ██║██████╔╝█████╗
██║╚██╗██║██╔══╝  ██║   ██║██║███╗██║██╔═══╝ ██╔══╝
██║ ╚████║███████╗╚██████╔╝╚███╔███╔╝██║     ██║
╚═╝  ╚═══╝╚══════╝ ╚═════╝  ╚══╝╚══╝ ╚═╝     ╚═╝

WPF-Inspired UI framework for Modern C++
Love from Envy!! :3

============================================================
*/

#pragma once

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Platform/Window.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/UI.h>
#include <NeoWPF/Input/Input.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>

namespace NeoWPF {

    using Core::Application;

    using Platform::Window;

    using UI::Panel;

    using UI::StackPanel;

    using UI::Grid;

    using UI::Border;

    using UI::TextBlock;

    using UI::TextBox;

    using UI::Image;
    
    namespace Media {
        using ::NeoWPF::UI::Core::Media::BitmapImage;
        using ::NeoWPF::UI::Core::Media::Stretch;
        using ::NeoWPF::UI::Core::Media::StretchDirection;
    }

    using UI::Button;

    using UI::Checkbox;

    using UI::ItemsControl;

    using UI::Content::ContentControl;

    using UI::Content::ContentPresenter;

    using UI::ScrollBar;

    using Input::InputManager;

    namespace Input {

        using ::NeoWPF::Input::Core::MouseEventArgs;

        using ::NeoWPF::Input::Core::KeyboardEventArgs;

        using ::NeoWPF::Input::Core::RoutedEventArgs;

        using ::NeoWPF::Input::Core::RoutedEvent;

        using ::NeoWPF::Input::Core::RoutingStrategy;

        using ::NeoWPF::Input::Core::MouseEventHandler;

        using ::NeoWPF::Input::Core::KeyboardEventHandler;

        using ::NeoWPF::Input::Core::RoutedEventHandler;

        using ::NeoWPF::Input::Core::RoutedMouseEventArgs;

        using ::NeoWPF::Input::Core::RoutedKeyboardEventArgs;

        using ::NeoWPF::Input::Core::MouseDownEvent;

        using ::NeoWPF::Input::Core::MouseUpEvent;

        using ::NeoWPF::Input::Core::MouseMoveEvent;

        using ::NeoWPF::Input::Core::MouseEnterEvent;

        using ::NeoWPF::Input::Core::MouseLeaveEvent;

        using ::NeoWPF::Input::Core::KeyDownEvent;

        using ::NeoWPF::Input::Core::KeyUpEvent;

    }

}