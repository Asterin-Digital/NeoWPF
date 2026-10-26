#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/ApplicationHost.h>
#include <NeoWPF/UI/Controls/Window.h>

#include <deque>
#include <memory>
#include <string>

namespace {

class TestWindowing final : public NeoWPF::Contracts::IWindowing {
public:
    void* CreateWindow(const wchar_t*, int, int, std::shared_ptr<NeoWPF::UI::Window>) override
    {
        return nullptr;
    }

    bool GetNativeWindowInfo(void*, NeoWPF::Contracts::NativeWindowInfo&) const noexcept override
    {
        return false;
    }

    bool IsWindowAlive(void*) const noexcept override { return false; }

    void GetWindowSize(void*, unsigned int& outWidth, unsigned int& outHeight) const noexcept override
    {
        outWidth = 0;
        outHeight = 0;
    }

    void ResizeWindow(void*, int, int) noexcept override {}
    void ShowWindow(void*) noexcept override {}
    void HideWindow(void*) noexcept override {}
    void CloseWindow(void*) noexcept override {}
    void InvalidateWindow(void*) noexcept override {}

    bool PumpMessages(std::chrono::milliseconds) override
    {
        ++pumpCount;
        if (pumpResults.empty()) {
            return true;
        }
        const bool result = pumpResults.front();
        pumpResults.pop_front();
        return result;
    }

    int ExitCode() const noexcept override { return exitCode; }

    void RequestQuit(int code) noexcept override
    {
        requestedExitCode = code;
        exitCode = code;
    }

    void ShowFatalError(const std::wstring&, const std::wstring&) noexcept override {}
    void SetConsoleVisible(bool) noexcept override {}
    bool IsConsoleVisible() const noexcept override { return false; }

    std::deque<bool> pumpResults;
    int exitCode = 0;
    int requestedExitCode = -1;
    int pumpCount = 0;
};

TEST(DesktopApplicationHostTests, DoesNotRunAFrameAfterMessagePumpRequestsExit)
{
    TestWindowing windowing;
    NeoWPF::Core::ApplicationSession session;
    windowing.exitCode = 12;
    windowing.pumpResults.push_back(false);
    NeoWPF::Core::DesktopApplicationHost host(windowing, session);
    int frameCount = 0;

    const int exitCode = host.Run([&frameCount] {
        ++frameCount;
        return true;
    });

    EXPECT_EQ(frameCount, 0);
    EXPECT_EQ(windowing.pumpCount, 1);
    EXPECT_EQ(exitCode, 12);
}

TEST(DesktopApplicationHostTests, PumpsBeforeEachFrameAndStopsWhenFrameRequestsExit)
{
    TestWindowing windowing;
    NeoWPF::Core::ApplicationSession session;
    windowing.exitCode = 23;
    NeoWPF::Core::DesktopApplicationHost host(windowing, session);
    int frameCount = 0;

    const int exitCode = host.Run([&frameCount] {
        return ++frameCount < 3;
    });

    EXPECT_EQ(frameCount, 3);
    EXPECT_EQ(windowing.pumpCount, 3);
    EXPECT_EQ(exitCode, 23);
}

TEST(DesktopApplicationHostTests, RequestQuitForwardsExitCodeToWindowing)
{
    TestWindowing windowing;
    NeoWPF::Core::ApplicationSession session;
    NeoWPF::Core::DesktopApplicationHost host(windowing, session);

    host.RequestQuit(42);

    EXPECT_EQ(windowing.requestedExitCode, 42);
    EXPECT_EQ(windowing.ExitCode(), 42);
    EXPECT_FALSE(host.IsShutdown());
}

TEST(DesktopApplicationHostTests, RunDeactivatesHostWithoutShuttingItDown)
{
    TestWindowing windowing;
    NeoWPF::Core::ApplicationSession session;
    NeoWPF::Core::DesktopApplicationHost host(windowing, session);
    int frameCount = 0;

    EXPECT_EQ(host.Run([&] {
        EXPECT_TRUE(host.IsActive());
        return ++frameCount < 2;
    }), 0);

    EXPECT_EQ(frameCount, 2);
    EXPECT_FALSE(host.IsActive());
    EXPECT_FALSE(host.IsShutdown());
    EXPECT_EQ(host.Run([&] { return ++frameCount < 3; }), 0);
    EXPECT_EQ(frameCount, 3);
}

TEST(DesktopApplicationHostTests, ShutdownPreventsActivationAndFrames)
{
    TestWindowing windowing;
    NeoWPF::Core::ApplicationSession session;
    NeoWPF::Core::DesktopApplicationHost host(windowing, session);
    int frameCount = 0;

    host.Shutdown();

    EXPECT_TRUE(host.IsShutdown());
    EXPECT_FALSE(host.IsActive());
    EXPECT_EQ(host.Run([&] {
        ++frameCount;
        return true;
    }), 0);
    EXPECT_EQ(frameCount, 0);
    EXPECT_EQ(windowing.pumpCount, 0);
}

TEST(DesktopApplicationHostTests, WaitsForFirstWindowButExitsAfterLastWindowCloses)
{
    EXPECT_TRUE(NeoWPF::Core::ShouldContinueDesktopLoop(false, false));
    EXPECT_TRUE(NeoWPF::Core::ShouldContinueDesktopLoop(true, true));
    EXPECT_FALSE(NeoWPF::Core::ShouldContinueDesktopLoop(true, false));
}

TEST(ApplicationSessionTests, PreservesContentAcrossPauseAndResume)
{
    auto contentHost = std::make_shared<NeoWPF::UI::Window>();
    NeoWPF::Core::ApplicationSession session(contentHost);

    EXPECT_TRUE(session.Activate());
    session.Deactivate();
    EXPECT_EQ(session.GetContentHost(), contentHost);
    EXPECT_TRUE(session.Activate());
    EXPECT_EQ(session.GetContentHost(), contentHost);
}

TEST(ApplicationSessionTests, MainWindowPropertyAndSessionStaySynchronized)
{
    NeoWPF::Core::Application application;
    auto firstContentHost = std::make_shared<NeoWPF::UI::Window>();
    auto replacementContentHost = std::make_shared<NeoWPF::UI::Window>();

    application.SetMainWindow(firstContentHost);

    EXPECT_EQ(application.GetMainWindow(), firstContentHost);
    EXPECT_EQ(
        application.GetValue<std::shared_ptr<NeoWPF::UI::Window>>(
            NeoWPF::Core::Application::MainWindowProperty()),
        firstContentHost);

    application.SetValue(
        NeoWPF::Core::Application::MainWindowProperty(), replacementContentHost);

    EXPECT_EQ(application.GetMainWindow(), replacementContentHost);
    application.SetMainWindow({});
}

TEST(ApplicationSessionTests, MainWindowIsUsedForPrimaryDesktopWindowOnly)
{
    auto mainWindow = std::make_shared<NeoWPF::UI::Window>();
    auto explicitWindow = std::make_shared<NeoWPF::UI::Window>();
    NeoWPF::Core::ApplicationSession session(mainWindow);

    EXPECT_EQ(session.SelectContentHost({}, true), mainWindow);
    EXPECT_EQ(session.SelectContentHost(explicitWindow, true), explicitWindow);
    EXPECT_FALSE(session.SelectContentHost({}, false));
}

TEST(SessionLifecycleTests, CopiesObserveTheSameStateTransitions)
{
    NeoWPF::Core::SessionLifecycle lifecycle;
    auto sessionCopy = lifecycle;

    EXPECT_TRUE(lifecycle.Activate());
    EXPECT_TRUE(sessionCopy.IsActive());
    sessionCopy.Shutdown();
    EXPECT_TRUE(lifecycle.IsShutdown());
    EXPECT_FALSE(lifecycle.Activate());
}

TEST(ApplicationSessionTests, ShutdownPreventsReactivationUntilNewRunBegins)
{
    NeoWPF::Core::ApplicationSession session;

    session.Shutdown();

    EXPECT_TRUE(session.IsShutdown());
    EXPECT_FALSE(session.Activate());
    session.Begin();
    EXPECT_FALSE(session.IsShutdown());
    EXPECT_TRUE(session.Activate());
}

}
