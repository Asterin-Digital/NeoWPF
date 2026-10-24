#include <NeoWPF/Core/Application/ApplicationHost.h>

#include <chrono>

namespace NeoWPF::Core {

DesktopApplicationHost::DesktopApplicationHost(NeoWPF::Contracts::IWindowing& windowing) noexcept
    : m_windowing(windowing)
{
}

int DesktopApplicationHost::Run(const FrameCallback& frameCallback)
{
    if (!frameCallback) {
        return m_windowing.ExitCode();
    }

    constexpr auto frameInterval = std::chrono::milliseconds(16);
    auto nextFrame = std::chrono::steady_clock::now();
    while (true) {
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

    return m_windowing.ExitCode();
}

void DesktopApplicationHost::RequestQuit(int exitCode) noexcept
{
    m_windowing.RequestQuit(exitCode);
}

}
