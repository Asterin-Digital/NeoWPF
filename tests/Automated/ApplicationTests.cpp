#include <gtest/gtest.h>

#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Diagnostics/PerformanceMetrics.h>

#include <chrono>

namespace {

class LifecycleTestApplication final : public NeoWPF::Core::Application {
public:
    int initializeCount = 0;
    int quitCount = 0;

protected:
    void InitializeOnLoad() override { ++initializeCount; }
    void OnApplicationQuit() override { ++quitCount; }
};

TEST(ApplicationTests, LifecycleHooksRunOncePerLifecycle)
{
    auto application = std::make_shared<LifecycleTestApplication>();
    NeoWPF::Core::Application::SetCurrent(application);

    NeoWPF::Core::Application::Initialize();
    EXPECT_EQ(application->initializeCount, 1);
    EXPECT_EQ(application->quitCount, 0);

    NeoWPF::Core::Application::Shutdown();
    EXPECT_EQ(application->quitCount, 1);

    NeoWPF::Core::Application::Initialize();
    EXPECT_EQ(application->initializeCount, 2);

    NeoWPF::Core::Application::Shutdown();
    EXPECT_EQ(application->quitCount, 2);
    NeoWPF::Core::Application::SetCurrent(nullptr);
}

TEST(ApplicationTests, ApplicationStartsUnder700msWithColdStartup)
{
    NeoWPF::Diagnostics::PerformanceMetrics::SetEnabled(true);
    NeoWPF::Diagnostics::PerformanceMetrics::SetStartupActionTrackingEnabled(true);

    const auto start = std::chrono::steady_clock::now();
    NeoWPF::Core::Application::Initialize();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();

    EXPECT_LT(elapsedMs, 700);

    NeoWPF::Diagnostics::PerformanceMetrics::RecordStartup();
    EXPECT_GE(NeoWPF::Diagnostics::PerformanceMetrics::GetStartupElapsedSeconds(), 0.0);

    NeoWPF::Core::Application::Shutdown();
}

TEST(ApplicationTests, StartupActionTrackingCapturesInitializationTiming)
{
    NeoWPF::Diagnostics::PerformanceMetrics::SetEnabled(true);
    NeoWPF::Diagnostics::PerformanceMetrics::SetStartupActionTrackingEnabled(true);

    NeoWPF::Core::Application::Initialize();

    const double initializationSeconds =
        NeoWPF::Diagnostics::PerformanceMetrics::GetStartupActionTotalSeconds("Application.Initialize");

    EXPECT_GT(initializationSeconds, 0.0);

    NeoWPF::Core::Application::Shutdown();
}

TEST(ApplicationTests, StartupMetricsRecordAStableAndNonNegativeValue)
{
    NeoWPF::Diagnostics::PerformanceMetrics::SetEnabled(true);
    NeoWPF::Diagnostics::PerformanceMetrics::SetStartupActionTrackingEnabled(true);

    NeoWPF::Core::Application::Initialize();
    NeoWPF::Diagnostics::PerformanceMetrics::RecordStartup();

    EXPECT_TRUE(NeoWPF::Diagnostics::PerformanceMetrics::HasStartupRecorded());
    EXPECT_GE(NeoWPF::Diagnostics::PerformanceMetrics::GetStartupElapsedSeconds(), 0.0);
    EXPECT_LE(NeoWPF::Diagnostics::PerformanceMetrics::GetStartupElapsedSeconds(), 1.0);

    NeoWPF::Core::Application::Shutdown();
}

} // namespace
