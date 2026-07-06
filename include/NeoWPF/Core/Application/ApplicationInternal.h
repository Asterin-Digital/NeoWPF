#pragma once

#include <Windows.h>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Core/Application/Application.h>

namespace NeoWPF::Diagnostics {
struct VisualTreeDumpOptions;
}

namespace NeoWPF::UI {
class Visual;
}

namespace NeoWPF::Core {

struct WindowState {
    HWND hwnd = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    bool needsLayout = true;
};

struct AppState {
    HINSTANCE hInst = nullptr;
    std::vector<WindowState> windows;
    std::atomic<bool> running{false};
    std::shared_ptr<NeoWPF::UI::Visual> developerWarningOverlayHost;
    std::atomic<bool> renderThreadInitialized{false};
    std::atomic<bool> renderThreadInitFailed{false};
    std::thread renderThread;
    int exitCode = 0;

    std::mutex renderInitMutex;
    std::condition_variable renderInitCondition;

    std::mutex windowsMutex;
    std::mutex commandListMutex;
    NeoWPF::Rendering::CommandList renderCommandList;
    NeoWPF::Rendering::CommandList buildCommandList;
    bool commandListInitialized = false;
    std::vector<NeoWPF::Diagnostics::DeveloperWarningMessage> pendingDeveloperMessages;
    std::vector<std::shared_ptr<NeoWPF::UI::Visual>> developerOverlayVisuals;
};

AppState& GetAppState();
void StartRenderThread();
HWND CreateApplicationWindow(const wchar_t* title, int width, int height);
int RunApplicationMessageLoop();
void ShutdownApplication();
void DumpVisualTreeImpl(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options);

}
