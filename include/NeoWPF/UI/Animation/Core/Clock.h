/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

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
