/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace NeoWPF::Diagnostics::PerformanceMetrics {

void Initialize();
void Shutdown();
void SetEnabled(bool enabled);
bool IsEnabled();

void SetStartupActionTrackingEnabled(bool enabled);
bool IsStartupActionTrackingEnabled() noexcept;
void RecordStartupAction(const char* actionName, double elapsedSeconds);
void EmitStartupActionSummary();

void SetActionTimingEnabled(bool enabled);
bool IsActionTimingEnabled() noexcept;
void RecordAction(const char* actionName, double elapsedSeconds);

void RecordStartup();
bool HasStartupRecorded() noexcept;
double GetStartupElapsedSeconds() noexcept;
double GetStartupActionTotalSeconds(const char* actionName);
void RecordFrameSample(double elapsedSeconds);
void RecordLayoutPass(double elapsedSeconds);
void RecordRenderPass(double elapsedSeconds);
void RecordMessageLoopTick(double elapsedSeconds);
void RecordWindowCreate();
void RecordWindowClose();
void SampleSystemMetrics();
void EmitPeriodicSummary();
void ForceSummary();

class StartupActionTimer {
public:
    explicit StartupActionTimer(const char* actionName) noexcept
        : m_actionName(actionName), m_active(IsStartupActionTrackingEnabled())
    {
        if (m_active) {
            m_startTime = std::chrono::steady_clock::now();
        }
    }

    ~StartupActionTimer() noexcept
    {
        if (!m_active) {
            return;
        }

        const auto elapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - m_startTime).count();
        RecordStartupAction(m_actionName, elapsedSeconds);
    }

private:
    const char* m_actionName;
    std::chrono::steady_clock::time_point m_startTime;
    bool m_active;
};

class ActionTimer {
public:
    explicit ActionTimer(const char* actionName) noexcept
        : m_actionName(actionName), m_active(IsActionTimingEnabled())
    {
        if (m_active) {
            m_startTime = std::chrono::steady_clock::now();
        }
    }

    ~ActionTimer() noexcept
    {
        if (!m_active) {
            return;
        }

        const auto elapsedSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - m_startTime).count();
        RecordAction(m_actionName, elapsedSeconds);
    }

private:
    const char* m_actionName;
    std::chrono::steady_clock::time_point m_startTime;
    bool m_active;
};

#define NEO_WPF_CONCAT_IMPL(a, b) a##b
#define NEO_WPF_CONCAT(a, b) NEO_WPF_CONCAT_IMPL(a, b)

#define NEO_WPF_STARTUP_ACTION(actionName) \
    NeoWPF::Diagnostics::PerformanceMetrics::StartupActionTimer NEO_WPF_CONCAT(_neoWpfStartupActionTimer, __COUNTER__)(actionName)

#define NEO_WPF_ACTION(actionName) \
    NeoWPF::Diagnostics::PerformanceMetrics::ActionTimer NEO_WPF_CONCAT(_neoWpfActionTimer, __COUNTER__)(actionName)

}
