#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Entry.h>
#include <NeoWPF/Core/Application/EntryRunner.h>
#include <NeoWPF/Core/Application/LaunchContext.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

#include <functional>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::function<int(const NeoWPF::Core::LaunchContext&)> s_entry;
bool s_applicationHostWasShutdown = false;

class TestApplicationHost final : public NeoWPF::Core::IApplicationHost {
public:
    int Run(const FrameCallback&) override
    {
        return 0;
    }

    void RequestQuit(int) noexcept override
    {
    }

    void Shutdown() noexcept override
    {
        s_applicationHostWasShutdown = true;
    }

    bool IsActive() const noexcept override
    {
        return false;
    }

    bool IsShutdown() const noexcept override
    {
        return s_applicationHostWasShutdown;
    }
};

}

int NeoWPF::Core::Entry(const NeoWPF::Core::LaunchContext& launch)
{
    return s_entry(launch);
}

TEST(EntryRunnerTests, PassesLaunchResultsAndReturnsTheApplicationCode)
{
    const std::vector<std::wstring> expectedArguments{L"app", L"--test"};
    const std::filesystem::path expectedDirectory{"launch-directory"};
    bool entryCalled = false;
    s_entry = [&](const NeoWPF::Core::LaunchContext& launch) {
        entryCalled = true;
        EXPECT_EQ(launch.Arguments().Value(), expectedArguments);
        EXPECT_TRUE(launch.DataUri().Success());
        EXPECT_FALSE(launch.DataUri().Value());
        EXPECT_EQ(launch.ExecutableDirectory().Value(), expectedDirectory);
        return 37;
    };

    const int exitCode = NeoWPF::Core::EntryRunner::Run(
        NeoWPF::Success<std::vector<std::wstring>>{expectedArguments},
        NeoWPF::Success<std::optional<std::string>>{std::nullopt},
        NeoWPF::Success<std::filesystem::path>{expectedDirectory});

    EXPECT_TRUE(entryCalled);
    EXPECT_EQ(exitCode, 37);
}

TEST(EntryRunnerTests, PreservesLaunchFailuresForTheApplicationEntry)
{
    bool receivedArgumentFailure = false;
    s_entry = [&](const NeoWPF::Core::LaunchContext& launch) {
        receivedArgumentFailure = launch.Arguments().Fail();
        if (receivedArgumentFailure) {
            EXPECT_EQ(launch.Arguments().Error(), "argument conversion failed");
        }
        EXPECT_EQ(launch.ExecutableDirectory().Value(), std::filesystem::path{});
        return 0;
    };

    const int exitCode = NeoWPF::Core::EntryRunner::Run(
        NeoWPF::Fail<std::string>{"argument conversion failed"},
        NeoWPF::Success<std::optional<std::string>>{std::nullopt},
        NeoWPF::Success<std::filesystem::path>{std::filesystem::path{}});

    EXPECT_TRUE(receivedArgumentFailure);
    EXPECT_EQ(exitCode, 0);
}

TEST(EntryRunnerTests, ReturnsFailureWhenTheApplicationEntryThrows)
{
    s_entry = [](const NeoWPF::Core::LaunchContext&) -> int {
        throw std::runtime_error("entry failure");
    };

    EXPECT_EQ(
        NeoWPF::Core::EntryRunner::Run(
            NeoWPF::Success<std::vector<std::wstring>>{{}},
            NeoWPF::Success<std::optional<std::string>>{std::nullopt},
            NeoWPF::Success<std::filesystem::path>{std::filesystem::path{}}),
        1);
}

TEST(EntryRunnerTests, ClearsProcessStateBeforeTheNextEntryInvocation)
{
    s_applicationHostWasShutdown = false;
    auto firstWindow = std::make_shared<NeoWPF::UI::Window>();
    auto firstApplication = std::make_shared<NeoWPF::Core::Application>();
    auto* const firstApplicationAddress = firstApplication.get();
    int invocation = 0;
    auto& input = NeoWPF::Input::InputManager::GetInstance();

    s_entry = [&](const NeoWPF::Core::LaunchContext&) {
        ++invocation;
        if (invocation == 1) {
            NeoWPF::Core::Application::SetCurrent(firstApplication);
            NeoWPF::Core::Application::Initialize();
            firstApplication->SetMainWindow(firstWindow);
            input.SetRootElement(firstWindow.get());
            input.SetFocusedElement(firstWindow.get());
            input.SetOverlayElements({firstWindow});
            EXPECT_TRUE(input.SetPointerCapture(19, firstWindow.get()));
            input.ProcessKeyboardInput(40, true);

            auto& appState = NeoWPF::Core::GetAppState();
            appState.developerWarningOverlayHost = firstWindow;
            appState.textInspectionOverlayHost = firstWindow;
            appState.developerOverlayVisuals.push_back(firstWindow);
            appState.textInspectionOverlayVisuals.push_back(firstWindow);
            appState.pendingTextInspectionLines.push_back(L"stale text inspection");
            appState.renderThreadInitFailed = true;
            appState.layoutDebugLogging = true;
            appState.running = true;
            appState.exitCode = 23;
            appState.applicationHost = std::make_unique<TestApplicationHost>();
            {
                std::lock_guard<std::mutex> lock(appState.renderRequestMutex);
                appState.renderRequested = true;
            }
            return 0;
        }

        NeoWPF::Core::Application::Initialize();
        EXPECT_NE(&NeoWPF::Core::Application::Current(), firstApplicationAddress);
        EXPECT_FALSE(NeoWPF::Core::Application::Current().GetMainWindow());
        EXPECT_EQ(input.GetFocusedElement(), nullptr);
        EXPECT_EQ(input.GetPointerCapture(19), nullptr);
        EXPECT_FALSE(input.IsKeyDown(40));
        EXPECT_FALSE(input.HitTest(0.0, 0.0));
        EXPECT_FALSE(input.SetPointerCapture(19, firstWindow.get()));

        auto& appState = NeoWPF::Core::GetAppState();
        EXPECT_FALSE(appState.applicationSession.GetContentHost());
        EXPECT_TRUE(appState.windowSessions.empty());
        EXPECT_FALSE(appState.hasCreatedWindow);
        EXPECT_FALSE(appState.developerWarningOverlayHost);
        EXPECT_FALSE(appState.textInspectionOverlayHost);
        EXPECT_TRUE(appState.developerOverlayVisuals.empty());
        EXPECT_TRUE(appState.textInspectionOverlayVisuals.empty());
        EXPECT_TRUE(appState.pendingTextInspectionLines.empty());
        EXPECT_FALSE(appState.running.load());
        EXPECT_FALSE(appState.renderThreadInitialized.load());
        EXPECT_FALSE(appState.renderThreadInitFailed.load());
        EXPECT_FALSE(appState.layoutDebugLogging.load());
        EXPECT_EQ(appState.exitCode, 0);
        {
            std::lock_guard<std::mutex> lock(appState.applicationHostMutex);
            EXPECT_FALSE(appState.applicationHost);
        }
        {
            std::lock_guard<std::mutex> lock(appState.renderRequestMutex);
            EXPECT_FALSE(appState.renderRequested);
        }
        return 0;
    };

    const auto run = [] {
        return NeoWPF::Core::EntryRunner::Run(
            NeoWPF::Success<std::vector<std::wstring>>{{}},
            NeoWPF::Success<std::optional<std::string>>{std::nullopt},
            NeoWPF::Success<std::filesystem::path>{std::filesystem::path{}});
    };

    EXPECT_EQ(run(), 0);
    EXPECT_EQ(run(), 0);
    EXPECT_EQ(invocation, 2);
    EXPECT_TRUE(s_applicationHostWasShutdown);
}
