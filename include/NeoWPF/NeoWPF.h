#pragma once

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Platform/Window.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/UI.h>
#include <NeoWPF/Input/Input.h>

namespace NeoWPF {

    /*
        @summary
        Application lifecycle manager.

        @details
        Provides initialization, the main run loop, shutdown and access to
        the current application instance. Typical startup sequence:
        call `Application::Initialize()` then `Application::Run()`.

        Example:
            NeoWPF::Application::Initialize();
            return NeoWPF::Application::Run();

        @since 1.0
    */
    using Core::Application;

    /*
        @summary
        Platform window wrapper (native HWND on Windows).

        @details
        Thin convenience wrapper around the native window handle which
        provides creation helpers and basic callbacks (resize, visibility,
        etc.). This represents the OS-level window; the UI `UI::Window`
        is a separate visual control that can be hosted in the visual tree.

        @since 1.0
    */
    using Platform::Window;

    /*
        @summary
        Generic panel base used for building layout containers.

        @details
        `Panel` is the lightweight foundation for layout containers such as
        StackPanel and Grid. Use it when you need a background or a simple
        container for child elements. Panels participate in measure/arrange
        passes and report desired size accordingly.

        @since 1.0
    */
    using UI::Panel;

    /*
        @summary
        Vertical or horizontal stack layout.

        @details
        Lay out children sequentially along one axis. Configure orientation
        and spacing via the control's properties. Use for simple column or
        row-like layouts when Grid semantics are unnecessary.

        @since 1.0
    */
    using UI::StackPanel;

    /*
        @summary
        Flexible grid layout with row/column sizing modes.

        @details
        Supports Auto, Fixed and Star sizing for rows and columns. Define
        dimensions with AddRow/AddColumn and place children using AddChild
        or Grid::SetRow/SetColumn. Auto sized rows/columns measure children
        with infinite space on the respective axis and use the children's
        desired sizes when arranging.

        Example:
            auto grid = std::make_shared<NeoWPF::Grid>();
            grid->AddRow(NeoWPF::UI::SizeMode::Auto, 0);
            grid->AddRow(NeoWPF::UI::SizeMode::Star, 1);
            grid->AddColumn(NeoWPF::UI::SizeMode::Star, 1);
            grid->AddChild(child.get(), 0, 0);

        @since 1.0
    */
    using UI::Grid;

    /*
        @summary
        Lightweight text control for displaying read-only strings.

        @details
        Use TextBlock to render simple text. Configure font size, color and
        wrapping via the control's public properties. TextBlock participates
        in measure/arrange and reports its desired size based on measured
        text metrics.

        @since 1.0
    */
    using UI::TextBlock;

    /*
        @summary
        Push button control with a Click event.

        @details
        Button is an interactive control that raises a Click event. Subscribe
        with `+=` to handle presses. Configure appearance via background,
        padding and template APIs. The control measures its content and
        reports an appropriate desired size.

        Example:
            auto btn = std::make_shared<NeoWPF::Button>(L"OK");
            btn->Click += [](){ // handler };

        @since 1.0
    */
    using UI::Button;

    /*
        @summary
        Central input dispatcher and hit-test helper.

        @details
        InputManager processes raw platform input, performs hit-testing
        against the visual tree, manages focus and mouse capture. Call
        ProcessMouseInput/ProcessKeyboardInput from the platform layer or
        rely on the provided platform glue.

        @since 1.0
    */
    using Input::InputManager;

    namespace Input {

        /*
            @summary
            Mouse input event arguments (position, buttons, wheel).

            @since 1.0
        */
        using Core::MouseEventArgs;

        /*
            @summary
            Keyboard input event arguments (key code, char, down/up).

            @since 1.0
        */
        using Core::KeyboardEventArgs;

        /*
            @summary
            Represents a routed event declaration (tunnel/bubble/direct).

            @since 1.0
        */
        using Core::RoutedEvent;

        /*
            @summary
            Routing strategy for routed events.

            @since 1.0
        */
        using Core::RoutingStrategy;

        /*
            @summary
            Mouse event handler type alias.

            @since 1.0
        */
        using Core::MouseEventHandler;

        /*
            @summary
            Keyboard event handler type alias.

            @since 1.0
        */
        using Core::KeyboardEventHandler;

    }

}
