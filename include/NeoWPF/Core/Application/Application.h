#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <any>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/Core/Application/Resources/ResourceManager.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>
#include <NeoWPF/Diagnostics/TextDumper.h>
#include <NeoWPF/Platform/NativeWindow.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/IXamlResourceHost.h>
#include <typeindex>

#undef CreateWindow

namespace NeoWPF::UI {
class FrameworkElement;
class Panel;
class Window;
}

namespace NeoWPF::Core {

class Application : public NeoWPF::UI::Properties::DependencyObject,
                    public NeoWPF::UI::Markup::Xaml::IXamlResourceHost {
public:
    static const NeoWPF::UI::Properties::DependencyProperty& MainWindowProperty();

    Application() = default;

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
        Sets whether the application console should be visible.

        @param visible
        True to show the console, false to hide it.
    */
    static void ShowConsole(bool visible);

    /*
        @summary
        Returns whether the application console is currently visible.

        @returns
        True if the console is visible, otherwise false.
    */
    static bool IsConsoleVisible();

    /*
        @summary
        Shuts down the application and cleans up runtime resources.
    */
    static void Shutdown();

    /*
        @summary
        Returns the current application instance.

        @returns
        A reference to the active Application singleton.
    */
    static Application& Current();

    /*
        @summary
        Installs the application instance used by the runtime.

        @param application
        The application instance to make current. Passing an empty shared pointer
        clears the current instance.
    */
    static void SetCurrent(std::shared_ptr<Application> application);

    /*
        @summary
        Sets the application's main window.

        @param window
        The window associated with this application.
    */
    void SetMainWindow(const std::shared_ptr<NeoWPF::UI::Window>& window) { m_mainWindow = window; }

    /*
        @summary
        Returns the application's main window.

        @returns
        A reference to the stored main-window shared pointer.
    */
    const std::shared_ptr<NeoWPF::UI::Window>& GetMainWindow() const noexcept { return m_mainWindow; }

    /*
        @summary
        Stores a resource value in the application resource dictionary.

        @param key
        The resource key.

        @param value
        The resource value to store.
    */
    static void SetResource(const std::string& key, std::any value);

    /*
        @summary
        Stores a keyed resource loaded from XAML.

        @param key
        The resource key.

        @param value
        The resource value.

        @returns
        True when the resource was stored.

        @internal
    */
    bool SetXamlResource(const std::string& key, const std::any& value) override;

    /*
        @summary
        Stores an implicit style loaded from XAML for a target type.

        @param targetType
        The type the style targets.

        @param value
        The style value.

        @returns
        True when the style was stored.

        @internal
    */
    bool SetImplicitXamlStyle(
        const std::type_index& targetType,
        const std::shared_ptr<NeoWPF::UI::Styling::Style>& style) override;

    /*
        @summary
        Attempts to find a resource value by key.

        @param key
        The resource key.

        @param outValue
        The output value if the resource exists.

        @returns
        True if the resource was found, otherwise false.
    */
    static bool TryFindResource(const std::string& key, std::any& outValue);

    /*
        @summary
        Looks up a keyed resource loaded from XAML.

        @param key
        The resource key.

        @param outValue
        Receives the resource value when found.

        @returns
        True when the resource exists.

        @internal
    */
    bool TryFindXamlResource(const std::string& key, std::any& outValue) const override;

    /*
        @summary
        Looks up an implicit style loaded from XAML for a target type.

        @param targetType
        The type the style targets.

        @param outValue
        Receives the style value when found.

        @returns
        True when a matching style exists.

        @internal
    */
    bool TryFindImplicitStyle(
        const std::type_index& targetType,
        std::shared_ptr<NeoWPF::UI::Styling::Style>& outStyle) const override;

    /*
        @summary
        Finds a resource value by key.

        @param key
        The resource key.

        @returns
        The resource value if found, otherwise an empty std::any.
    */
    static std::any FindResource(const std::string& key);

    /*
        @summary
        Attempts to retrieve a resource value by key.

        @param key
        The resource key.

        @param outValue
        The output value if the resource exists.

        @returns
        True if the resource was found, otherwise false.
    */
    static bool TryGetResource(const std::string& key, std::any& outValue);

    /*
        @summary
        Returns the runtime resource manager used by the application.
    */
    NeoWPF::ResourceManager& Resources();

    /*
        @summary
        Loads a font file for use by the text renderer.

        @param filePath
        The path to a .ttf or .otf font file available to the app.

        @returns
        True if the font was registered successfully.
    */
    static bool LoadFontFromFile(const std::wstring& filePath);

    /*
        @summary
        Loads an embedded font from memory for use by the text renderer.

        @param data
        The font bytes.

        @param size
        The number of bytes in the font.

        @param familyName
        The family name used by controls, normally the font filename without its extension.

        @returns
        True if the font was registered successfully.
    */
    static bool LoadFontFromMemory(
        const unsigned char* data,
        size_t size,
        const std::wstring& familyName);

    /*
        @summary
        Creates a new application window.

        @param title
        The window title.

        @param width
        The initial window width.

        @param height
        The initial window height.

        @returns
        The created window.
    */
    NeoWPF::Platform::Window CreateWindow(const wchar_t* title, int width, int height);

    /*
        @summary
        Creates a native window using an existing UI window host.

        @param title
        The native window title.

        @param width
        The initial window width.

        @param height
        The initial window height.

        @param windowHost
        The UI window that owns the content and title-bar state.

        @returns
        The created native window.
    */
    NeoWPF::Platform::Window CreateWindow(
        const wchar_t* title,
        int width,
        int height,
        const std::shared_ptr<NeoWPF::UI::Window>& windowHost);

    /*
        @summary
        Creates a native window from an existing UI window host.

        @param windowHost
        The UI window that owns the content and title-bar state.

        @param width
        The initial window width.

        @param height
        The initial window height.

        @returns
        The created native window.
    */
    NeoWPF::Platform::Window CreateWindow(
        const std::shared_ptr<NeoWPF::UI::Window>& windowHost,
        int width,
        int height);

    /*
        @summary
        Enables or disables global layout inspection.

        @param enabled
        True to log measure/arrange results for framework elements.

        @internal
    */
    static void SetLayoutInspection(bool enabled);

    /*
        @summary
        Returns whether layout inspection is enabled.

        @returns
        True when layout measure/arrange state is being reported.

        @internal
    */
    static bool IsLayoutInspectionEnabled();

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
        Dumps the current visual tree with layout metadata enabled.

        @internal
    */
    static void DumpLayoutTree();

    /*
        @summary
        Dumps all text-bearing controls in the current visual tree.

        @internal
    */
    static void DumpTextTree(const NeoWPF::Diagnostics::TextDumpOptions& options = NeoWPF::Diagnostics::TextDumpOptions{});

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

private:
    std::shared_ptr<NeoWPF::UI::Window> m_mainWindow;
    static std::shared_ptr<Application> s_current;
};

}
