#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
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
        session.lifecycle.Activate();
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

TEST(ApplicationWindowSessionTests, SurfaceCopiesShareGenerationInvalidation)
{
    int nativeHandle = 0;
    WindowSessionGuard session(&nativeHandle);
    NeoWPF::Rendering::RenderSurface surface;
    {
        auto& appState = NeoWPF::Core::GetAppState();
        std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
        const auto found = std::find_if(appState.windowSessions.begin(), appState.windowSessions.end(),
            [&nativeHandle](const NeoWPF::Core::WindowSession& current) {
                return current.renderSurface.nativeWindowHandle == &nativeHandle;
            });
        ASSERT_NE(found, appState.windowSessions.end());
        surface = found->renderSurface;
    }
    const auto renderSnapshot = surface;
    ASSERT_TRUE(surface.IsValid());
    ASSERT_TRUE(renderSnapshot.IsValid());

    EXPECT_TRUE(NeoWPF::Core::CloseApplicationWindow(&nativeHandle));

    EXPECT_FALSE(surface.IsValid());
    EXPECT_FALSE(renderSnapshot.IsValid());
}

TEST(ApplicationWindowSessionTests, NewSurfaceGetsDistinctGeneration)
{
    int nativeHandle = 0;
    const NeoWPF::Rendering::RenderSurface first{&nativeHandle};
    const NeoWPF::Rendering::RenderSurface replacement{&nativeHandle};

    EXPECT_NE(first.GetGenerationId(), replacement.GetGenerationId());
}

TEST(ApplicationWindowSessionTests, SurfaceDestructionInvalidatesGeneration)
{
    int nativeHandle = 0;
    const NeoWPF::Rendering::RenderSurface surface{&nativeHandle};

    auto destruction = surface.LockForDestruction();

    EXPECT_TRUE(destruction.owns_lock());
    EXPECT_FALSE(surface.IsValid());
}

TEST(ApplicationWindowSessionTests, InvalidationWaitsForActiveRenderUse)
{
    int nativeHandle = 0;
    NeoWPF::Rendering::RenderSurface surface{&nativeHandle};
    auto surfaceUse = surface.TryAcquireUse();
    ASSERT_TRUE(surfaceUse.owns_lock());

    std::promise<void> invalidationStarted;
    std::promise<void> invalidationCompleted;
    auto completed = invalidationCompleted.get_future();
    std::thread invalidator([&surface, &invalidationStarted, &invalidationCompleted] {
        invalidationStarted.set_value();
        surface.Invalidate();
        invalidationCompleted.set_value();
    });
    invalidationStarted.get_future().wait();

    EXPECT_EQ(completed.wait_for(std::chrono::milliseconds(10)), std::future_status::timeout);
    surfaceUse = {};
    EXPECT_EQ(completed.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    invalidator.join();
    EXPECT_FALSE(surface.IsValid());
    EXPECT_FALSE(surface.TryAcquireUse().owns_lock());
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

TEST(ApplicationWindowSessionTests, ClosingOneWindowShutsDownOnlyItsHostSession)
{
    int primaryHandle = 0;
    int secondaryHandle = 0;
    WindowSessionGuard primarySession(&primaryHandle);
    WindowSessionGuard secondarySession(&secondaryHandle);
    auto& appState = NeoWPF::Core::GetAppState();
    NeoWPF::Rendering::RenderSurface secondarySurface;
    NeoWPF::Core::SessionLifecycle secondaryLifecycle;
    {
        std::lock_guard<std::mutex> lock(appState.windowSessionsMutex);
        auto primary = std::find_if(appState.windowSessions.begin(), appState.windowSessions.end(),
            [&primaryHandle](const NeoWPF::Core::WindowSession& session) {
                return session.renderSurface.nativeWindowHandle == &primaryHandle;
            });
        auto secondary = std::find_if(appState.windowSessions.begin(), appState.windowSessions.end(),
            [&secondaryHandle](const NeoWPF::Core::WindowSession& session) {
                return session.renderSurface.nativeWindowHandle == &secondaryHandle;
            });
        ASSERT_NE(primary, appState.windowSessions.end());
        ASSERT_NE(secondary, appState.windowSessions.end());
        secondarySurface = secondary->renderSurface;
        secondaryLifecycle = secondary->lifecycle;
    }

    EXPECT_TRUE(NeoWPF::Core::CloseApplicationWindow(&primaryHandle));

    EXPECT_FALSE(HasWindowSession(&primaryHandle));
    EXPECT_TRUE(secondaryLifecycle.IsActive());
    EXPECT_TRUE(secondarySurface.IsValid());
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
