#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/ApplicationHost.h>

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
    windowing.exitCode = 12;
    windowing.pumpResults.push_back(false);
    NeoWPF::Core::DesktopApplicationHost host(windowing);
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
    windowing.exitCode = 23;
    NeoWPF::Core::DesktopApplicationHost host(windowing);
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
    NeoWPF::Core::DesktopApplicationHost host(windowing);

    host.RequestQuit(42);

    EXPECT_EQ(windowing.requestedExitCode, 42);
    EXPECT_EQ(windowing.ExitCode(), 42);
    EXPECT_FALSE(host.IsShutdown());
}

TEST(DesktopApplicationHostTests, RunDeactivatesHostWithoutShuttingItDown)
{
    TestWindowing windowing;
    NeoWPF::Core::DesktopApplicationHost host(windowing);
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
    NeoWPF::Core::DesktopApplicationHost host(windowing);
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

}
