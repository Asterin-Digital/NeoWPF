/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Window.h>

namespace NeoWPF::Diagnostics {
struct VisualTreeDumpOptions;
}

namespace NeoWPF::UI {
class Visual;
}

namespace NeoWPF::Core {

struct WindowState {
    void* nativeWindowHandle = nullptr;
    unsigned int width = 0;
    unsigned int height = 0;
    bool needsLayout = true;
    bool isPrimaryWindow = false;
    std::shared_ptr<NeoWPF::UI::Window> windowHost;
    NeoWPF::UI::Visual* contentRoot = nullptr;
    NeoWPF::Rendering::CommandList buildCommandList;
    NeoWPF::Rendering::CommandList renderCommandList;
    bool commandListInitialized = false;
};

struct AppState {
    void* hInst = nullptr;
    std::vector<WindowState> windows;
    std::atomic<bool> running{false};
    std::atomic<bool> consoleVisible{false};
    std::shared_ptr<NeoWPF::UI::Visual> developerWarningOverlayHost;
    std::shared_ptr<NeoWPF::UI::Visual> textInspectionOverlayHost;
    std::atomic<bool> renderThreadInitialized{false};
    std::atomic<bool> renderThreadInitFailed{false};
    std::atomic<bool> layoutDebugLogging{false};
    std::thread renderThread;
    int exitCode = 0;

    std::mutex renderInitMutex;
    std::condition_variable renderInitCondition;
    std::mutex renderRequestMutex;
    std::condition_variable renderRequestCondition;
    bool renderRequested = false;

    std::mutex windowsMutex;
    std::mutex commandListMutex;
    NeoWPF::Rendering::CommandList renderCommandList;
    NeoWPF::Rendering::CommandList buildCommandList;
    bool commandListInitialized = false;
    std::vector<NeoWPF::Diagnostics::DeveloperWarningMessage> pendingDeveloperMessages;
    std::vector<std::shared_ptr<NeoWPF::UI::Visual>> developerOverlayVisuals;
    std::vector<std::wstring> pendingTextInspectionLines;
    std::vector<std::shared_ptr<NeoWPF::UI::Visual>> textInspectionOverlayVisuals;
};

AppState& GetAppState();
void StartRenderThread();
bool CloseApplicationWindow(void* nativeWindowHandle);
int RunApplicationMessageLoop();
void ShutdownApplication();
void DumpVisualTreeImpl(const NeoWPF::Diagnostics::VisualTreeDumpOptions& options);
void SetInputRootForWindow(void* nativeWindowHandle);

}
