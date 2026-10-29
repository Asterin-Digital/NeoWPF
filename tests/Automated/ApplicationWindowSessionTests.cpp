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

class SessionGuard {
public:
    explicit SessionGuard(void* handle)
        : m_handle(handle)
    {
        NeoWPF::Core::WindowSession session;
        session.renderSurface.nativeWindowHandle = handle;
        session.lifecycle.Activate();
        session.windowHost = std::make_shared<NeoWPF::UI::Window>();

        auto& app = NeoWPF::Core::GetAppState();
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        app.windowSessions.push_back(std::move(session));
    }

    ~SessionGuard()
    {
        NeoWPF::Core::CloseWindow(m_handle);
    }

private:
    void* m_handle;
};

bool HasSession(void* handle)
{
    auto& app = NeoWPF::Core::GetAppState();
    std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
    return std::any_of(app.windowSessions.begin(), app.windowSessions.end(),
        [handle](const NeoWPF::Core::WindowSession& session) {
            return session.renderSurface.nativeWindowHandle == handle;
        });
}

TEST(SessionTests, CloseRejectsNull)
{
    EXPECT_FALSE(NeoWPF::Core::CloseWindow(nullptr));
}

TEST(SessionTests, CopiesShareInvalidation)
{
    int handle = 0;
    SessionGuard session(&handle);
    NeoWPF::Rendering::RenderSurface surface;
    {
        auto& app = NeoWPF::Core::GetAppState();
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        const auto found = std::find_if(app.windowSessions.begin(), app.windowSessions.end(),
            [&handle](const NeoWPF::Core::WindowSession& current) {
                return current.renderSurface.nativeWindowHandle == &handle;
            });
        ASSERT_NE(found, app.windowSessions.end());
        surface = found->renderSurface;
    }
    const auto snapshot = surface;
    ASSERT_TRUE(surface.IsValid());
    ASSERT_TRUE(snapshot.IsValid());

    EXPECT_TRUE(NeoWPF::Core::CloseWindow(&handle));

    EXPECT_FALSE(surface.IsValid());
    EXPECT_FALSE(snapshot.IsValid());
}

TEST(SessionTests, NewSurfaceGetsFreshGeneration)
{
    int handle = 0;
    const NeoWPF::Rendering::RenderSurface first{&handle};
    const NeoWPF::Rendering::RenderSurface next{&handle};

    EXPECT_NE(first.GetGenerationId(), next.GetGenerationId());
}

TEST(SessionTests, SurfaceReattachGetsFreshGeneration)
{
    int handle = 0;
    SessionGuard session(&handle);
    NeoWPF::Rendering::RenderSurface original;
    NeoWPF::Core::SessionLifecycle life;
    {
        auto& app = NeoWPF::Core::GetAppState();
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        const auto found = std::find_if(app.windowSessions.begin(), app.windowSessions.end(),
            [&handle](const NeoWPF::Core::WindowSession& current) {
                return current.renderSurface.nativeWindowHandle == &handle;
            });
        ASSERT_NE(found, app.windowSessions.end());
        original = found->renderSurface;
        life = found->lifecycle;
    }

    ASSERT_TRUE(NeoWPF::Core::ReleaseWindowSurface(&handle));

    EXPECT_TRUE(HasSession(&handle));
    EXPECT_FALSE(original.IsValid());
    EXPECT_FALSE(life.IsActive());
    EXPECT_TRUE(NeoWPF::Core::AttachWindowSurface(&handle));

    NeoWPF::Rendering::RenderSurface replacement;
    {
        auto& app = NeoWPF::Core::GetAppState();
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        const auto found = std::find_if(app.windowSessions.begin(), app.windowSessions.end(),
            [&handle](const NeoWPF::Core::WindowSession& current) {
                return current.renderSurface.nativeWindowHandle == &handle;
            });
        ASSERT_NE(found, app.windowSessions.end());
        replacement = found->renderSurface;
    }
    EXPECT_TRUE(replacement.IsValid());
    EXPECT_NE(original.GetGenerationId(), replacement.GetGenerationId());
    EXPECT_TRUE(life.IsActive());
}

TEST(SessionTests, DestroyInvalidatesGeneration)
{
    int handle = 0;
    const NeoWPF::Rendering::RenderSurface surface{&handle};

    auto destroy = surface.LockForDestruction();

    EXPECT_TRUE(destroy.owns_lock());
    EXPECT_FALSE(surface.IsValid());
}

TEST(SessionTests, InvalidationWaitsForRenderUse)
{
    int handle = 0;
    NeoWPF::Rendering::RenderSurface surface{&handle};
    auto use = surface.TryAcquireUse();
    ASSERT_TRUE(use.owns_lock());

    std::promise<void> started;
    std::promise<void> finished;
    auto complete = finished.get_future();
    std::thread worker([&surface, &started, &finished] {
        started.set_value();
        surface.Invalidate();
        finished.set_value();
    });
    started.get_future().wait();

    EXPECT_EQ(complete.wait_for(std::chrono::milliseconds(10)), std::future_status::timeout);
    use = {};
    EXPECT_EQ(complete.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    worker.join();
    EXPECT_FALSE(surface.IsValid());
    EXPECT_FALSE(surface.TryAcquireUse().owns_lock());
}

TEST(SessionTests, CloseRejectsUnknownHandle)
{
    int registered = 0;
    int unknown = 0;
    SessionGuard session(&registered);

    EXPECT_FALSE(NeoWPF::Core::CloseWindow(&unknown));
    EXPECT_TRUE(HasSession(&registered));
}

TEST(SessionTests, CloseRemovesSessionOnce)
{
    int handle = 0;
    SessionGuard session(&handle);

    EXPECT_TRUE(NeoWPF::Core::CloseWindow(&handle));
    EXPECT_FALSE(HasSession(&handle));
    EXPECT_FALSE(NeoWPF::Core::CloseWindow(&handle));
}

TEST(SessionTests, ClosingOneKeepsOtherActive)
{
    int first = 0;
    int second = 0;
    SessionGuard firstSession(&first);
    SessionGuard secondSession(&second);
    auto& app = NeoWPF::Core::GetAppState();
    NeoWPF::Rendering::RenderSurface secondSurface;
    NeoWPF::Core::SessionLifecycle secondLife;
    {
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        auto firstIt = std::find_if(app.windowSessions.begin(), app.windowSessions.end(),
            [&first](const NeoWPF::Core::WindowSession& session) {
                return session.renderSurface.nativeWindowHandle == &first;
            });
        auto secondIt = std::find_if(app.windowSessions.begin(), app.windowSessions.end(),
            [&second](const NeoWPF::Core::WindowSession& session) {
                return session.renderSurface.nativeWindowHandle == &second;
            });
        ASSERT_NE(firstIt, app.windowSessions.end());
        ASSERT_NE(secondIt, app.windowSessions.end());
        secondSurface = secondIt->renderSurface;
        secondLife = secondIt->lifecycle;
    }

    EXPECT_TRUE(NeoWPF::Core::CloseWindow(&first));

    EXPECT_FALSE(HasSession(&first));
    EXPECT_TRUE(secondLife.IsActive());
    EXPECT_TRUE(secondSurface.IsValid());
}

TEST(SessionTests, CloseQueuesSurfaceDestroyAfterStartup)
{
    int handle = 0;
    SessionGuard session(&handle);
    auto& app = NeoWPF::Core::GetAppState();
    const bool wasInitialized = app.renderThreadInitialized.exchange(true);
    const bool wasRunning = app.running.exchange(true);
    {
        std::lock_guard<std::mutex> lock(app.renderRequestMutex);
        app.renderRequested = false;
    }
    std::atomic<bool> destroyQueued{false};
    std::thread renderer([&app, &handle, &destroyQueued] {
        {
            std::unique_lock<std::mutex> lock(app.renderRequestMutex);
            app.renderRequestCondition.wait(lock, [&app] {
                return app.renderRequested;
            });
            app.renderRequested = false;
        }
        std::vector<std::shared_ptr<NeoWPF::Core::SurfaceDestroyRequest>> requests;
        {
            std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
            requests.swap(app.pendingSurfaceDestroy);
        }
        for (const auto& request : requests) {
            destroyQueued = request->surface.nativeWindowHandle == &handle;
            NeoWPF::Rendering::Renderer_DestroySurface(request->surface);
            {
                std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
                request->completed = true;
            }
            app.surfaceDestroyCondition.notify_all();
        }
    });

    EXPECT_TRUE(NeoWPF::Core::CloseWindow(&handle));
    renderer.join();
    EXPECT_TRUE(destroyQueued.load());
    {
        std::lock_guard<std::mutex> lock(app.windowSessionsMutex);
        EXPECT_TRUE(app.pendingSurfaceDestroy.empty());
    }
    app.renderThreadInitialized = wasInitialized;
    app.running = wasRunning;
    {
        std::lock_guard<std::mutex> lock(app.renderRequestMutex);
        app.renderRequested = false;
    }
}

}
