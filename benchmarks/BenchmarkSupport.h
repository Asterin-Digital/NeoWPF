/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>
#include <memory>
#include <string>

namespace NeoWPF::UI {
class StackPanel;
}

namespace NeoWPF::Benchmarks {

inline constexpr std::size_t kStackChildCount = 256;
inline constexpr double kLayoutWidth = 1024.0;
inline constexpr double kLayoutHeight = 4096.0;
inline constexpr double kTextWidth = 560.0;
inline constexpr double kTextHeight = 240.0;
inline constexpr auto kBenchmarkFontFamily = L"NeoWPF Benchmark Font";

/*
    @summary
    Registers the embedded font used by text benchmarks.

    @internal
*/
bool RegisterBenchmarkFont();

/*
    @summary
    Creates the shared panel fixture used by layout, rendering, and input benchmarks.

    @internal
*/
std::shared_ptr<NeoWPF::UI::StackPanel> CreateStackWorkload();

/*
    @summary
    Creates repeated representative text for text benchmarks.

    @internal
*/
std::wstring CreateTextWorkload(std::size_t repetitionCount);

}