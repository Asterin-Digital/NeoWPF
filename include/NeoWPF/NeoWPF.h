////////////////////////////////////////////////////////////
// Documentation is AI-Generated to save myself some time
// If you notice any mistakes or bad example, please tell me
// so I can quickly fix it! :>
////////////////////////////////////////////////////////////

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
        the current application instance. Typical startup: call
        `Application::Initialize()` then `Application::Run()`.

        @example
        NeoWPF::Application::Initialize();
        return NeoWPF::Application::Run();

        @since 1.0
    */

    using Core::Application;


    /*
        @summary
        Platform window wrapper (native `HWND` on Windows).

        @details
        Thin convenience wrapper around the native window handle which
        provides creation helpers and basic callbacks (resize, etc.). Use
        `Platform::Window::CreateWindow(...)` to obtain a platform window.

        @note
        This type represents the OS window; the UI-level `UI::Window`
        control is a separate consumable that can be hosted in the visual
        tree.

        @since 1.0
    */

    using Platform::Window;


    /*
        @summary
        Generic panel base used for building layout containers.

        @details
        `Panel` is the base class for layout containers (StackPanel, Grid,
        etc.). Use it when you need a lightweight container for child
        elements and backgrounds.

        @since 1.0
    */

    using UI::Panel;


    /*
        @summary
        Vertical or horizontal stack layout.

        @details
        Lay out children sequentially in one axis. Configure orientation via
        the control's properties (see `StackPanel` API). Use for simple
        column/row-like layouts where complex grid behavior is unnecessary.

        @since 1.0
    */

    using UI::StackPanel;


    /*
        @summary
        Flexible grid layout with row/column sizing modes.

        @details
        Supports `Auto`, `Fixed` and star (`*`) sizing for rows and columns.
        Use `Grid` when you need multi-dimensional layouts. Attach row and
        column indexes to children using the Grid helpers.

        @example
        // Create a 2x2 grid and place children using SetRow/SetColumn.

        @since 1.0
    */

    using UI::Grid;


    /*
        @summary
        Lightweight text control for displaying read-only strings.

        @details
        Use `TextBlock` to render simple text. Configure font size, color and
        wrapping via the control's public properties. `TextBlock` participates
        in measure/arrange and reports its desired size.

        @since 1.0
    */

    using UI::TextBlock;


    /*
        @summary
        Push button control with a `Click` event.

        @details
        `Button` is an interactive control raising a public `Click` event.
        Subscribe with `+=` to handle presses. Configure appearance via
        background, padding and template APIs.

        @example
        NeoWPF::Button* b = new NeoWPF::Button();
        b->SetText(L"OK");
        b->Click += [](auto&&){ // handler };

        @since 1.0
    */

    using UI::Button;


    /*
        @summary
        Central input dispatcher and hit-test helper.

        @details
        `InputManager` processes raw platform input, performs hit-testing
        against the visual tree, manages focus and mouse capture. Call
        `ProcessMouseInput` / `ProcessKeyboardInput` from your platform
        integration layer, or rely on the provided platform glue which does
        this for you.

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
