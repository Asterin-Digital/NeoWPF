#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace {

class WindowSessionGuard {
public:
    explicit WindowSessionGuard(void* nativeWindowHandle)
        : m_nativeWindowHandle(nativeWindowHandle)
    {
        NeoWPF::Core::WindowSession session;
        session.renderSurface.nativeWindowHandle = nativeWindowHandle;
        session.windowHost = std::make_shared<NeoWPF::UI::Window>();

        auto& appState = NeoWPF::Core::GetAppState();
        std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
        appState.windowSessions.push_back(std::move(session));
    }

    ~WindowSessionGuard()
    {
        NeoWPF::Core::CloseApplicationWindow(m_nativeWindowHandle);
    }

private:
    void* m_nativeWindowHandle;
};

bool HasWindowSession(void* nativeWindowHandle)
{
    auto& appState = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
    return std::any_of(appState.windowSessions.begin(), appState.windowSessions.end(),
        [nativeWindowHandle](const NeoWPF::Core::WindowSession& session) {
            return session.renderSurface.nativeWindowHandle == nativeWindowHandle;
        });
}

TEST(ApplicationWindowSessionTests, CloseRejectsNullHandle)
{
    EXPECT_FALSE(NeoWPF::Core::CloseApplicationWindow(nullptr));
}

TEST(ApplicationWindowSessionTests, CloseRejectsUnknownHandleAndPreservesRegisteredSession)
{
    int registeredHandle = 0;
    int unknownHandle = 0;
    WindowSessionGuard session(&registeredHandle);

    EXPECT_FALSE(NeoWPF::Core::CloseApplicationWindow(&unknownHandle));
    EXPECT_TRUE(HasWindowSession(&registeredHandle));
}

TEST(ApplicationWindowSessionTests, CloseRemovesRegisteredSessionOnlyOnce)
{
    int nativeHandle = 0;
    WindowSessionGuard session(&nativeHandle);

    EXPECT_TRUE(NeoWPF::Core::CloseApplicationWindow(&nativeHandle));
    EXPECT_FALSE(HasWindowSession(&nativeHandle));
    EXPECT_FALSE(NeoWPF::Core::CloseApplicationWindow(&nativeHandle));
}

TEST(ApplicationWindowSessionTests, CloseQueuesRendererSurfaceReleaseAfterRendererStartup)
{
    int nativeHandle = 0;
    WindowSessionGuard session(&nativeHandle);
    auto& appState = NeoWPF::Core::GetAppState();
    const bool wasRendererInitialized = appState.renderThreadInitialized.exchange(true);
    const bool wasRunning = appState.running.exchange(true);
    {
        std::lock_guard<std::mutex> lock(appState.renderRequestMutex);
        appState.renderRequested = false;
    }
    std::atomic<bool> surfaceReleaseRequested{false};
    std::thread renderer([&appState, &nativeHandle, &surfaceReleaseRequested] {
        {
            std::unique_lock<std::mutex> lock(appState.renderRequestMutex);
            appState.renderRequestCondition.wait(lock, [&appState] {
                return appState.renderRequested;
            });
            appState.renderRequested = false;
        }
        std::vector<std::shared_ptr<NeoWPF::Core::RenderSurfaceDestructionRequest>> requests;
        {
            std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
            requests.swap(appState.pendingRenderSurfaceDestruction);
        }
        for (const auto& request : requests) {
            surfaceReleaseRequested = request->surface.nativeWindowHandle == &nativeHandle;
            NeoWPF::Rendering::Renderer_DestroySurface(request->surface);
            {
                std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
                request->completed = true;
            }
            appState.renderSurfaceDestructionCondition.notify_all();
        }
    });

    EXPECT_TRUE(NeoWPF::Core::CloseApplicationWindow(&nativeHandle));
    renderer.join();
    EXPECT_TRUE(surfaceReleaseRequested.load());
    {
        std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
        EXPECT_TRUE(appState.pendingRenderSurfaceDestruction.empty());
    }
    appState.renderThreadInitialized = wasRendererInitialized;
    appState.running = wasRunning;
    {
        std::lock_guard<std::mutex> lock(appState.renderRequestMutex);
        appState.renderRequested = false;
    }
}

}
