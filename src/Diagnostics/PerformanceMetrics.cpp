#include <NeoWPF/Diagnostics/PerformanceMetrics.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <atomic>
#include <algorithm>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace NeoWPF::Diagnostics::PerformanceMetrics {
namespace {

struct Counters {
    std::atomic<bool> enabled{true};
    std::atomic<bool> startupActionTrackingEnabled{false};
    std::atomic<bool> actionTimingEnabled{false};
    std::atomic<uint64_t> frameCount{0};
    std::atomic<uint64_t> layoutPassCount{0};
    std::atomic<uint64_t> renderPassCount{0};
    std::atomic<uint64_t> messageLoopTickCount{0};
    std::atomic<uint64_t> windowCreates{0};
    std::atomic<uint64_t> windowCloses{0};
    std::atomic<double> frameTimeAccumulator{0.0};
    std::atomic<double> layoutTimeAccumulator{0.0};
    std::atomic<double> renderTimeAccumulator{0.0};
    std::atomic<double> messageLoopTimeAccumulator{0.0};
    std::atomic<double> maxFrameTimeSeconds{0.0};
    std::atomic<double> maxLayoutTimeSeconds{0.0};
    std::atomic<double> maxRenderTimeSeconds{0.0};
    std::atomic<double> maxMessageLoopTimeSeconds{0.0};
    std::atomic<uint64_t> currentMemoryBytes{0};
    std::atomic<uint64_t> peakMemoryBytes{0};
    std::atomic<double> currentCpuUsage{0.0};
    std::atomic<double> peakCpuUsage{0.0};
    std::atomic<bool> initialized{false};
    std::atomic<bool> startupRecorded{false};
    std::atomic<double> startupElapsedSeconds{0.0};
    std::unordered_map<std::string, double> startupActionDurations;
    std::unordered_map<std::string, uint64_t> startupActionCounts;
    std::mutex startupActionMutex;
};

Counters& GetCounters()
{
    static Counters counters;
    return counters;
}

std::chrono::steady_clock::time_point& GetLastSummaryTime()
{
    static std::chrono::steady_clock::time_point last = std::chrono::steady_clock::now();
    return last;
}

std::chrono::steady_clock::time_point& GetStartupStartTime()
{
    static std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    return start;
}

void UpdateMax(std::atomic<double>& target, double value)
{
    double current = target.load(std::memory_order_relaxed);
    while (value > current && !target.compare_exchange_weak(current, value, std::memory_order_relaxed)) {
    }
}

template <typename T>
void AddAtomic(std::atomic<T>& target, T value)
{
    T current = target.load(std::memory_order_relaxed);
    while (!target.compare_exchange_weak(current, current + value, std::memory_order_relaxed)) {
    }
}

void EmitSummary(const char* label)
{
    auto& counters = GetCounters();
    const uint64_t frames = counters.frameCount.load(std::memory_order_relaxed);
    const uint64_t layouts = counters.layoutPassCount.load(std::memory_order_relaxed);
    const uint64_t renders = counters.renderPassCount.load(std::memory_order_relaxed);
    const uint64_t ticks = counters.messageLoopTickCount.load(std::memory_order_relaxed);
    const double frameAvg = frames > 0 ? counters.frameTimeAccumulator.load(std::memory_order_relaxed) / static_cast<double>(frames) : 0.0;
    const double layoutAvg = layouts > 0 ? counters.layoutTimeAccumulator.load(std::memory_order_relaxed) / static_cast<double>(layouts) : 0.0;
    const double renderAvg = renders > 0 ? counters.renderTimeAccumulator.load(std::memory_order_relaxed) / static_cast<double>(renders) : 0.0;
    const double tickAvg = ticks > 0 ? counters.messageLoopTimeAccumulator.load(std::memory_order_relaxed) / static_cast<double>(ticks) : 0.0;
    const uint64_t memMB = counters.currentMemoryBytes.load(std::memory_order_relaxed) / (1024 * 1024);
    const uint64_t peakMemMB = counters.peakMemoryBytes.load(std::memory_order_relaxed) / (1024 * 1024);
    const double cpuUsage = counters.currentCpuUsage.load(std::memory_order_relaxed);
    const double peakCpu = counters.peakCpuUsage.load(std::memory_order_relaxed);
    const double startupSeconds = counters.startupElapsedSeconds.load(std::memory_order_relaxed);

    NeoWPF::Diagnostics::LogInfo("[Perf] %s startup=%.4f frames=%llu layouts=%llu renders=%llu ticks=%llu creates=%llu closes=%llu avgFrame=%.4f avgLayout=%.4f avgRender=%.4f avgTick=%.4f maxFrame=%.4f maxLayout=%.4f maxRender=%.4f maxTick=%.4f mem=%lluMB peakMem=%lluMB cpu=%.1f%% peakCpu=%.1f%%",
        label,
        startupSeconds,
        static_cast<unsigned long long>(frames),
        static_cast<unsigned long long>(layouts),
        static_cast<unsigned long long>(renders),
        static_cast<unsigned long long>(ticks),
        static_cast<unsigned long long>(counters.windowCreates.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(counters.windowCloses.load(std::memory_order_relaxed)),
        frameAvg,
        layoutAvg,
        renderAvg,
        tickAvg,
        counters.maxFrameTimeSeconds.load(std::memory_order_relaxed),
        counters.maxLayoutTimeSeconds.load(std::memory_order_relaxed),
        counters.maxRenderTimeSeconds.load(std::memory_order_relaxed),
        counters.maxMessageLoopTimeSeconds.load(std::memory_order_relaxed),
        static_cast<unsigned long long>(memMB),
        static_cast<unsigned long long>(peakMemMB),
        cpuUsage,
        peakCpu);
}

}

void Initialize()
{
    auto& counters = GetCounters();
    if (counters.initialized.exchange(true, std::memory_order_acq_rel)) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    GetLastSummaryTime() = now;
    GetStartupStartTime() = now;
    counters.startupRecorded.store(false, std::memory_order_relaxed);
    counters.startupElapsedSeconds.store(0.0, std::memory_order_relaxed);
}

void Shutdown()
{
    ForceSummary();
}

void SetEnabled(bool enabled)
{
    GetCounters().enabled.store(enabled, std::memory_order_relaxed);
}

bool IsEnabled()
{
    return GetCounters().enabled.load(std::memory_order_relaxed);
}

void SetStartupActionTrackingEnabled(bool enabled)
{
    GetCounters().startupActionTrackingEnabled.store(enabled, std::memory_order_relaxed);
}

bool IsStartupActionTrackingEnabled() noexcept
{
    return GetCounters().startupActionTrackingEnabled.load(std::memory_order_relaxed);
}

void RecordStartupAction(const char* actionName, double elapsedSeconds)
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) ||
        !counters.initialized.load(std::memory_order_relaxed) ||
        !counters.startupActionTrackingEnabled.load(std::memory_order_relaxed)) {
        return;
    }

    std::lock_guard<std::mutex> lock(counters.startupActionMutex);
    counters.startupActionDurations[actionName] += elapsedSeconds;
    counters.startupActionCounts[actionName] += 1;
}

void EmitStartupActionSummary()
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) ||
        !counters.initialized.load(std::memory_order_relaxed) ||
        !counters.startupActionTrackingEnabled.load(std::memory_order_relaxed)) {
        return;
    }

    std::lock_guard<std::mutex> lock(counters.startupActionMutex);
    if (counters.startupActionDurations.empty()) {
        return;
    }

    NeoWPF::Diagnostics::LogInfo("[Perf] Startup action timing summary:");
    for (const auto& entry : counters.startupActionDurations) {
        const auto& name = entry.first;
        const double totalSeconds = entry.second;
        const uint64_t count = counters.startupActionCounts[name];
        const double avgSeconds = count > 0 ? totalSeconds / static_cast<double>(count) : 0.0;
        NeoWPF::Diagnostics::LogInfo("[Perf]   %s count=%llu total=%.4f avg=%.4f",
            name.c_str(), static_cast<unsigned long long>(count), totalSeconds, avgSeconds);
    }
}

void SetActionTimingEnabled(bool enabled)
{
    GetCounters().actionTimingEnabled.store(enabled, std::memory_order_relaxed);
}

bool IsActionTimingEnabled() noexcept
{
    return GetCounters().actionTimingEnabled.load(std::memory_order_relaxed);
}

void RecordAction(const char* actionName, double elapsedSeconds)
{
    auto& counters = GetCounters();
    if (!actionName ||
        !counters.enabled.load(std::memory_order_relaxed) ||
        !counters.initialized.load(std::memory_order_relaxed) ||
        !counters.actionTimingEnabled.load(std::memory_order_relaxed)) {
        return;
    }

    NeoWPF::Diagnostics::LogInfo("[Perf] action=%s elapsed=%.6f", actionName, elapsedSeconds);
}

void RecordStartup()
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) || !counters.initialized.load(std::memory_order_relaxed)) {
        return;
    }

    bool expected = false;
    if (!counters.startupRecorded.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return;
    }

    const double elapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - GetStartupStartTime()).count();
    counters.startupElapsedSeconds.store(elapsedSeconds, std::memory_order_relaxed);
    EmitSummary("startup");
    EmitStartupActionSummary();
}

bool HasStartupRecorded() noexcept
{
    return GetCounters().startupRecorded.load(std::memory_order_relaxed);
}

double GetStartupElapsedSeconds() noexcept
{
    return GetCounters().startupElapsedSeconds.load(std::memory_order_relaxed);
}

double GetStartupActionTotalSeconds(const char* actionName)
{
    if (!actionName) {
        return 0.0;
    }

    auto& counters = GetCounters();
    std::lock_guard<std::mutex> lock(counters.startupActionMutex);
    const auto it = counters.startupActionDurations.find(actionName);
    return it == counters.startupActionDurations.end() ? 0.0 : it->second;
}

void RecordFrameSample(double elapsedSeconds)
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) || !counters.initialized.load(std::memory_order_relaxed)) {
        return;
    }
    counters.frameCount.fetch_add(1, std::memory_order_relaxed);
    AddAtomic(counters.frameTimeAccumulator, elapsedSeconds);
    UpdateMax(counters.maxFrameTimeSeconds, elapsedSeconds);
}

void RecordLayoutPass(double elapsedSeconds)
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) || !counters.initialized.load(std::memory_order_relaxed)) {
        return;
    }
    counters.layoutPassCount.fetch_add(1, std::memory_order_relaxed);
    AddAtomic(counters.layoutTimeAccumulator, elapsedSeconds);
    UpdateMax(counters.maxLayoutTimeSeconds, elapsedSeconds);
}

void RecordRenderPass(double elapsedSeconds)
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) || !counters.initialized.load(std::memory_order_relaxed)) {
        return;
    }
    counters.renderPassCount.fetch_add(1, std::memory_order_relaxed);
    AddAtomic(counters.renderTimeAccumulator, elapsedSeconds);
    UpdateMax(counters.maxRenderTimeSeconds, elapsedSeconds);
}

void RecordMessageLoopTick(double elapsedSeconds)
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) || !counters.initialized.load(std::memory_order_relaxed)) {
        return;
    }
    counters.messageLoopTickCount.fetch_add(1, std::memory_order_relaxed);
    AddAtomic(counters.messageLoopTimeAccumulator, elapsedSeconds);
    UpdateMax(counters.maxMessageLoopTimeSeconds, elapsedSeconds);
}

void RecordWindowCreate()
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) || !counters.initialized.load(std::memory_order_relaxed)) {
        return;
    }
    counters.windowCreates.fetch_add(1, std::memory_order_relaxed);
}

void RecordWindowClose()
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) || !counters.initialized.load(std::memory_order_relaxed)) {
        return;
    }
    counters.windowCloses.fetch_add(1, std::memory_order_relaxed);
}

void EmitPeriodicSummary()
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) || !counters.initialized.load(std::memory_order_relaxed)) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration<double>(now - GetLastSummaryTime()).count();
    constexpr double summaryIntervalSeconds = 10.0;
    if (elapsed < summaryIntervalSeconds) {
        return;
    }

    GetLastSummaryTime() = now;
    SampleSystemMetrics();
    EmitSummary("periodic");
}

void ForceSummary()
{
    SampleSystemMetrics();
    EmitSummary("shutdown");
}

void SampleSystemMetrics()
{
    auto& counters = GetCounters();
    if (!counters.enabled.load(std::memory_order_relaxed) || !counters.initialized.load(std::memory_order_relaxed)) {
        return;
    }


    static std::clock_t lastCpuTime = 0;
    static std::chrono::steady_clock::time_point lastSample = std::chrono::steady_clock::now();

    const std::clock_t currentCpuTime = std::clock();
    const auto now = std::chrono::steady_clock::now();
    const auto timeDelta = std::chrono::duration<double>(now - lastSample).count();
    if (timeDelta > 0.0 && lastCpuTime != 0 && currentCpuTime >= lastCpuTime) {
            const double cpuSeconds = static_cast<double>(currentCpuTime - lastCpuTime) / CLOCKS_PER_SEC;
            double cpuUsage = cpuSeconds / timeDelta * 100.0;
            cpuUsage = std::min(cpuUsage, 100.0);

            counters.currentCpuUsage.store(cpuUsage, std::memory_order_relaxed);
            double peak = counters.peakCpuUsage.load(std::memory_order_relaxed);
            if (cpuUsage > peak) {
                counters.peakCpuUsage.store(cpuUsage, std::memory_order_relaxed);
            }
    }
    lastCpuTime = currentCpuTime;
    lastSample = now;
}

}
