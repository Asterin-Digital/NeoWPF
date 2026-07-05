#include <NeoWPF/UI/Animation/Core/Clock.h>

namespace NeoWPF::UI::Animation {

Clock::Clock() {
    Reset();
}

void Clock::Reset() noexcept {
    m_lastTick = std::chrono::steady_clock::now();
}

float Clock::Tick() noexcept {
    const auto now = std::chrono::steady_clock::now();
    const float elapsed = std::chrono::duration<float>(now - m_lastTick).count();
    m_lastTick = now;
    return elapsed;
}

}
