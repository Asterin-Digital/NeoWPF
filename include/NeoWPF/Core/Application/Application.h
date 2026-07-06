#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Diagnostics/VisualTreeDumper.h>


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

    static void Initialize();
    static int Run();
    static void Shutdown();
    static Application* Current();
    HWND CreateWindow(const wchar_t* title = L"NeoWPF", int width = 800, int height = 600);
    static void DumpVisualTree(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options = NeoWPF::Diagnostics::VisualTreeDumpOptions{});
    static void ShowDeveloperMessage(const std::wstring& title, const std::wstring& message, DeveloperMessageSeverity severity = DeveloperMessageSeverity::Warning);
    static void ClearDeveloperMessages();

private:
    Application() = default;
    static Application* s_current;
};

}
