#include <NeoWPF/Core/Application/ApplicationHost.h>

#include <chrono>

namespace NeoWPF::Core {

DesktopApplicationHost::DesktopApplicationHost(
    NeoWPF::Contracts::IWindowing& windowing,
    ApplicationSession& session) noexcept
    : m_windowing(windowing), m_session(session)
{
}

int DesktopApplicationHost::Run(const FrameCallback& frameCallback)
{
    if (!frameCallback || !m_session.Activate()) {
        return m_windowing.ExitCode();
    }

    constexpr auto frameInterval = std::chrono::milliseconds(16);
    auto nextFrame = std::chrono::steady_clock::now();
    try {
        while (!m_session.IsShutdown()) {
            const auto now = std::chrono::steady_clock::now();
            const auto timeout = now < nextFrame
                ? std::chrono::duration_cast<std::chrono::milliseconds>(nextFrame - now)
                : std::chrono::milliseconds(0);
            if (!m_windowing.PumpMessages(timeout)) {
                break;
            }

            nextFrame = std::chrono::steady_clock::now() + frameInterval;
            if (!frameCallback()) {
                break;
            }
        }
    } catch (...) {
        m_session.Deactivate();
        throw;
    }

    m_session.Deactivate();
    return m_windowing.ExitCode();
}

void DesktopApplicationHost::RequestQuit(int exitCode) noexcept
{
    m_windowing.RequestQuit(exitCode);
}

void DesktopApplicationHost::Shutdown() noexcept
{
    m_session.Shutdown();
}

bool DesktopApplicationHost::IsActive() const noexcept
{
    return m_session.IsActive();
}

bool DesktopApplicationHost::IsShutdown() const noexcept
{
    return m_session.IsShutdown();
}

}
