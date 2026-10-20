#include "BenchmarkSupport.h"

#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Layout/Placement/TextWrapping.h>

#include <benchmark/benchmark.h>

#include <cstdint>
#include <string>

namespace {

void BM_TextBlockMeasure(benchmark::State& state)
{
    const auto text = NeoWPF::Benchmarks::CreateTextWorkload(
        static_cast<std::size_t>(state.range(0)));
    NeoWPF::UI::TextBlock textBlock(text);
    textBlock.SetFontFamily(NeoWPF::Benchmarks::kBenchmarkFontFamily);
    textBlock.SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);
    textBlock.Measure(NeoWPF::Benchmarks::kTextWidth,
                      NeoWPF::Benchmarks::kTextHeight);
    for (auto _ : state) {
        static_cast<void>(_);
        textBlock.Measure(NeoWPF::Benchmarks::kTextWidth,
                          NeoWPF::Benchmarks::kTextHeight);
        benchmark::DoNotOptimize(textBlock.GetDesiredWidth());
    }
    state.SetItemsProcessed(
        state.iterations() * static_cast<std::int64_t>(text.size()));
}
BENCHMARK(BM_TextBlockMeasure)->Arg(8)->Arg(80)->Arg(400);

void BM_TextBlockMeasureChangingText(benchmark::State& state)
{
    const auto repetitionCount = static_cast<std::size_t>(state.range(0));
    const auto firstText = NeoWPF::Benchmarks::CreateTextWorkload(repetitionCount) + L"alpha";
    const auto secondText = NeoWPF::Benchmarks::CreateTextWorkload(repetitionCount) + L"beta";
    NeoWPF::UI::TextBlock textBlock(firstText);
    textBlock.SetFontFamily(NeoWPF::Benchmarks::kBenchmarkFontFamily);
    textBlock.SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);
    textBlock.Measure(NeoWPF::Benchmarks::kTextWidth,
                      NeoWPF::Benchmarks::kTextHeight);

    bool useSecondText = false;
    for (auto _ : state) {
        static_cast<void>(_);
        textBlock.SetText(useSecondText ? secondText : firstText);
        textBlock.Measure(NeoWPF::Benchmarks::kTextWidth,
                          NeoWPF::Benchmarks::kTextHeight);
        benchmark::DoNotOptimize(textBlock.GetDesiredWidth());
        useSecondText = !useSecondText;
    }
    state.SetItemsProcessed(
        state.iterations() * static_cast<std::int64_t>(firstText.size()));
}
BENCHMARK(BM_TextBlockMeasureChangingText)->Arg(8)->Arg(80);

void BM_TextBlockMeasureChangingWidth(benchmark::State& state)
{
    const auto text = NeoWPF::Benchmarks::CreateTextWorkload(
        static_cast<std::size_t>(state.range(0)));
    NeoWPF::UI::TextBlock textBlock(text);
    textBlock.SetFontFamily(NeoWPF::Benchmarks::kBenchmarkFontFamily);
    textBlock.SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);

    double width = NeoWPF::Benchmarks::kTextWidth;
    textBlock.Measure(width, NeoWPF::Benchmarks::kTextHeight);
    for (auto _ : state) {
        static_cast<void>(_);
        width = width == NeoWPF::Benchmarks::kTextWidth
            ? NeoWPF::Benchmarks::kTextWidth - 80.0
            : NeoWPF::Benchmarks::kTextWidth;
        textBlock.Measure(width, NeoWPF::Benchmarks::kTextHeight);
        benchmark::DoNotOptimize(textBlock.GetDesiredHeight());
    }
    state.SetItemsProcessed(
        state.iterations() * static_cast<std::int64_t>(text.size()));
}
BENCHMARK(BM_TextBlockMeasureChangingWidth)->Arg(8)->Arg(80);

}