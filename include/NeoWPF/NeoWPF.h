/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

/*
============================================================

███╗   ██╗███████╗ ██████╗ ██╗    ██╗██████╗ ███████╗
████╗  ██║██╔════╝██╔═══██╗██║    ██║██╔══██╗██╔════╝
██╔██╗ ██║█████╗  ██║   ██║██║ █╗ ██║██████╔╝█████╗
██║╚██╗██║██╔══╝  ██║   ██║██║███╗██║██╔═══╝ ██╔══╝
██║ ╚████║███████╗╚██████╔╝╚███╔███╔╝██║     ██║
╚═╝  ╚═══╝╚══════╝ ╚═════╝  ╚══╝╚══╝ ╚═╝     ╚═╝

A C++ UI framework meant to bring WPF familiarity to a fast, native and portable language.

Review the project's README.md, DevelopmentGuidelines.md and AI.md 
for details related to AI usage, contributing, and other information

============================================================
*/

#pragma once

#include <memory>
#include <utility>

///////////////////////////////////////////////////
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Core/Threading/BackgroundExecutor.h>
#include <NeoWPF/Core/Threading/Cancellation.h>
#include <NeoWPF/Core/Threading/MainThreadExecutor.h>
#include <NeoWPF/Core/Threading/Task.h>
#include <NeoWPF/Core/Threading/ThreadAffinity.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/UI.h>
#include <NeoWPF/Input/Input.h>
///////////////////////////////////////////////////
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/FailureHooks.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
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
// Media is not merged inside NeoWPF.Foundation. If you want to use media compile NeoWPF from scratch to get the library file
// or use NeoPM to turn NeoWPF.Media on for your project so you can use it.
//////////
// NeoWPF.Foundation media (like where bitmap image, colors) live in are separate from NeoWPF.Media
// This is to keep NeoWPF.Media (may contain heavy dependencies/features) that other people might not use
#if defined(NEOWPF_ENABLE_MEDIA) && NEOWPF_ENABLE_MEDIA
#include <NeoWPF/Media/Media.h>
#endif

// We recommend not including individual header files for public usage in your own code.
// NeoWPF reserves the right to update paths or method signatures for any implementation.
// Using this header file for public usage ensures that even if method signatures or the API
// change, you will receive shims, facades, etc. for API stability.

// Public API changes will happen, but they are gradual and will include deprecation warnings.
// NeoWPF reserves the right to update or otherwise remove a public API if we determine
// that it doesn't fit NeoWPF's architecture or design.

/*///////////////////////////////////////////////////////////////////////////////////////////*/

// For the project's actual documentation, please review docs/README.md and visit the website
// containing the full documentation.
// You can still read local header files for documentation, though they may be minimal in some places.

namespace NeoWPF {

    // These STD wrapper's aren't needed, if you don't need them use the macro (NEOWPF_NO_GLOBAL_USINGS) to turn them off
    // or comment them out inside the header file (NeoWPF.h). You should remember that updating NeoWPF.h (or the entire include folder)
    // will bring back the usings at the bottom, so you may need to re-comment them out again if you used this method.

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
    
    namespace Threading = Core::Threading;

    using Core::Application;

    using Core::ApplicationThemes::ThemeDefinition;

    using Core::ApplicationThemes::ThemeManager;

    using Platform::Window;

    using UI::Panel;

    using UI::StackPanel;

    using UI::Grid;

    using UI::Border;

    using UI::TextBlock;

    using UI::TextBox;

    using UI::Image;

    namespace Locales {
        using Platform::WideToUtf8;

        using Platform::Utf8ToWide;

        using Platform::Trim;

        using Platform::ToLower;

        using Platform::ToUpper;

        using Platform::NormalizeFileExtension;

        using Platform::IsAsciiAlpha;

        using Platform::IsAsciiDigit;

        using Platform::Split;

        using Platform::TryParseInt;

        using Platform::TryParseUInt32Hex;

        using ::NeoWPF::UI::Localization::Culture;

        using ::NeoWPF::UI::Localization::LocaleExpression;

        using ::NeoWPF::UI::Localization::LocaleOperations;

        using ::NeoWPF::UI::Localization::LocaleString;

        using ::NeoWPF::UI::Localization::LocaleTable;

        using ::NeoWPF::UI::Localization::LocalizationManager;
    }

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

    using UI::FilePicker;

    using UI::Checkbox;

    using UI::RadioButton;

    using UI::ItemsControl;

    using UI::ScrollViewer;

    using UI::Content::ContentControl;

    using UI::Content::ContentPresenter;

    using UI::ScrollBar;

    using UI::Slider;

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

    // shorter name, you may use Diagnostics as well but it's more verbose
    namespace Dev = ::NeoWPF::Diagnostics;

}

#ifndef NEOWPF_NO_GLOBAL_USINGS
//////////////////////////////////////////////////
// See details at the top if you don't need these
using NeoWPF::New;

using NeoWPF::Ref;

using NeoWPF::ToString;

using NeoWPF::Cast;

using NeoWPF::String;

//////////////////////////////////////////////////
#endif
