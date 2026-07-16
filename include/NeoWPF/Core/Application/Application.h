#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/Platform/Window.h>


#undef CreateWindow

namespace NeoWPF::UI {
class FrameworkElement;
class Panel;
}

namespace NeoWPF::Core {

class Application {
public:
    using DeveloperMessageSeverity = NeoWPF::Diagnostics::DeveloperWarningSeverity;

    struct DeveloperMessage {
        std::wstring title;
        std::wstring message;
        DeveloperMessageSeverity severity = DeveloperMessageSeverity::Warning;
    };

    /*
        @summary
        Initializes the NeoWPF application runtime.

        @details
        Call this before creating windows or entering the main loop.
    */
    static void Initialize();

    /*
        @summary
        Runs the application message loop.

        @returns
        The application exit code.
    */
    static int Run();

    /*
        @summary
        Shuts down the application and cleans up runtime resources.
    */
    static void Shutdown();

    /*
        @summary
        Returns the current application instance.

        @returns
        The active Application singleton, or nullptr if none is initialized.
    */
    static Application* Current();

    /*
        @summary
        Creates the main application window.

        @param title
        The window title.

        @param width
        The initial window width.

        @param height
        The initial window height.

        @returns
        The native HWND for the created window.
    */
    NeoWPF::Platform::Window CreateWindow(const wchar_t* title = L"NeoWPF", int width = 800, int height = 600);

    /*
        @summary
        Closes a previously created window.

        @param hwnd
        The native handle of the window to close.

        @returns
        True if the window was closed successfully, otherwise false.
    */
    bool CloseWindow(const NeoWPF::Platform::Window& window);

    /*
        @summary
        Dumps the current visual tree for debugging.

        @param options
        Options that control what data is included in the dump.

        @internal
    */
    static void DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options = NeoWPF::Diagnostics::VisualTreeDumpOptions{});

    /*
        @summary
        Shows a developer-facing message in the debug UI.

        @param title
        The message title.

        @param message
        The body text of the message.

        @param severity
        The severity level for the developer message.

        @internal 
    */
    static void ShowDeveloperMessage(const std::wstring& title, const std::wstring& message, DeveloperMessageSeverity severity = DeveloperMessageSeverity::Warning);

    /*
        @summary
        Clears all active developer messages.
        
        @internal
    */
    static void ClearDeveloperMessages();

public:
    static bool s_applicationDebugging;

private:
    Application() = default;
    static Application* s_current;
};

}
