#pragma once

#include <chrono>

namespace NeoWPF::UI::Animation {

class Clock {
public:
    Clock();

    void Reset() noexcept;
    float Tick() noexcept;

private:
    std::chrono::steady_clock::time_point m_lastTick;
};

}
