#pragma once

#include <Windows.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::Diagnostics {
struct VisualTreeDumpOptions;
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
};

AppState& GetAppState();
void StartRenderThread();
HWND CreateApplicationWindow(const wchar_t* title, int width, int height);
int RunApplicationMessageLoop();
void ShutdownApplication();
void DumpVisualTreeImpl(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options);

}
