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

Review the project's README for details.

============================================================
*/

#pragma once

#include <memory>
#include <utility>

///////////////////////////////////////////////////
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/UI.h>
#include <NeoWPF/Input/Input.h>
#include <NeoWPF/UI/Markup/Builder/Builder.h>
///////////////////////////////////////////////////
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/FailureHooks.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Diagnostics/TextDumper.h>
///////////////////////////////////////////////////
#include <NeoWPF/Core/Application/Resources/ResourceKey.h>
#include <NeoWPF/Core/Application/Resources/IResourceProvider.h>
#include <NeoWPF/Core/Application/Resources/EmbeddedResourceProvider.h>
#include <NeoWPF/Core/Application/Resources/ExternalFileResourceProvider.h>
#include <NeoWPF/Core/Application/Resources/ResourceManager.h>
#include <NeoWPF/UI/Markup/Xaml/Loading/CompiledXaml.h>
///////////////////////////////////////////////////

// We recommend not to include individual header files for public usage in your own code. 
// NeoWPF reserves the right to update paths or method signature's for any implementation.
// Using this header file for public usage ensure's that even if method signatures change, or the API changes
// you will get shims, facades, etc for API stability.

// Public API changes will happen, but they are gradual with deprecation warnings.
// NeoWPF reverses the right to update or otherwise remove a public API if we deem
// it doesn't fit NeoWPF's architecture or design.

namespace NeoWPF {

    // If you don't want to use these wrapper's you can just comment out the using's at the end of the file!! :>
    // These are only here to make code less verbose and cleaner

    /*
        @summary
        Type alias for std::shared_ptr<T>.

        @related New
    */
    template <typename T>
    using Ref = std::shared_ptr<T>;

    /*
        @summary
        Creates a new managed object and returns it as a shared_ptr.

        @param args
        Arguments forwarded to the constructor of T.

        @returns
        A shared_ptr to the newly constructed object.

        @example
        auto button = New<Button>();
        auto panel = New<StackPanel>(width, height);

        @related Ref
    */
    template <typename T, typename... Args>
    std::shared_ptr<T> New(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    /*
        @summary
        Converts a numeric value to a wide string representation.

        @param value
        +The value to convert.

        @returns
        A wide string representation of the value.

        @example
        auto text = ToString(42);  // L"42"
        auto text = ToString(3.14);  // L"3.140000"
    */
    template <typename T>
    std::wstring ToString(T value)
    {
        return std::to_wstring(value);
    }

    /*
        @summary
        Safely casts a std::any object to the specified type.

        @param value
        The std::any object to cast.

        @returns
        The casted value of type T.

        @warning
        Throws std::bad_any_cast if the stored type does not match T.

        @example
        auto num = Cast<int>(data);
        auto str = Cast<std::wstring>(data);
    */
    template <typename T>
    T Cast(const std::any& value)
    {
        return std::any_cast<T>(value);
    }

    /*
        @summary
        Creates a wide string from a wide character literal.

        @param str
        The wide character string literal.

        @returns
        A std::wstring containing the provided string.

        @example
        auto text = String(L"Hello World");
    */
    inline std::wstring String(const wchar_t* str)
    {
        return std::wstring(str);
    }

    inline std::wstring String(int value)
    {
        return std::to_wstring(value);
    }

    inline std::wstring String(unsigned int value)
    {
        return std::to_wstring(value);
    }

    inline std::wstring String(long value)
    {
        return std::to_wstring(value);
    }

    inline std::wstring String(unsigned long value)
    {
        return std::to_wstring(value);
    }

    inline std::wstring String(long long value)
    {
        return std::to_wstring(value);
    }

    inline std::wstring String(unsigned long long value)
    {
        return std::to_wstring(value);
    }
    using Core::Application;

    using Platform::Window;

    using UI::Panel;

    using UI::StackPanel;

    using UI::Grid;

    using UI::Border;

    using UI::TextBlock;

    using UI::TextBox;

    using UI::Image;

    namespace UI {
        namespace Media {
            using BitmapImage = ::NeoWPF::UI::Core::Media::BitmapImage;

            using Stretch = ::NeoWPF::UI::Core::Media::Stretch;

            using StretchDirection = ::NeoWPF::UI::Core::Media::StretchDirection;
        }

        namespace Triggers {
            using ::NeoWPF::UI::Styling::Triggers::TriggerBase;

            using ::NeoWPF::UI::Styling::Triggers::Setter;

            using ::NeoWPF::UI::Styling::Triggers::Trigger;

            using ::NeoWPF::UI::Styling::Triggers::DataTrigger;

            using ::NeoWPF::UI::Styling::Triggers::EventTrigger;

            using ::NeoWPF::UI::Styling::Triggers::TriggerCollection;

            using ::NeoWPF::UI::Styling::Triggers::TriggerAction;

            using ::NeoWPF::UI::Styling::Triggers::BeginStoryboardAction;
            
            using ::NeoWPF::UI::Styling::Triggers::StopStoryboardAction;
        }
    }

    using UI::Button;

    using UI::Checkbox;

    using UI::ItemsControl;

    using UI::ScrollViewer;

    using UI::Content::ContentControl;

    using UI::Content::ContentPresenter;

    using UI::ScrollBar;

    using Input::InputManager;

    using Input::Mouse;

    using Input::MouseButtonState;

    using Input::Keyboard;

    using Input::KeyState;

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

        using ::NeoWPF::Input::Core::PreviewMouseDownEvent;

        using ::NeoWPF::Input::Core::PreviewMouseUpEvent;

        using ::NeoWPF::Input::Core::PreviewMouseMoveEvent;

        using ::NeoWPF::Input::Core::PreviewMouseWheelEvent;

        using ::NeoWPF::Input::Core::MouseDownEvent;

        using ::NeoWPF::Input::Core::MouseUpEvent;

        using ::NeoWPF::Input::Core::MouseMoveEvent;

        using ::NeoWPF::Input::Mouse;

        using ::NeoWPF::Input::MouseButtonState;

        using ::NeoWPF::Input::Keyboard;

        using ::NeoWPF::Input::KeyState;

        using ::NeoWPF::Input::Core::MouseWheelEvent;

        using ::NeoWPF::Input::Core::MouseEnterEvent;

        using ::NeoWPF::Input::Core::MouseLeaveEvent;

        using ::NeoWPF::Input::Core::PreviewKeyDownEvent;

        using ::NeoWPF::Input::Core::PreviewKeyUpEvent;

        using ::NeoWPF::Input::Core::KeyDownEvent;

        using ::NeoWPF::Input::Core::KeyUpEvent;

    }

}

using NeoWPF::New;

using NeoWPF::Ref;

using NeoWPF::ToString;

using NeoWPF::Cast;

using NeoWPF::String;