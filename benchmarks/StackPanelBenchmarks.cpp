#include "BenchmarkSupport.h"

#include <NeoWPF/UI/Layout/Panel/StackPanel.h>

#include <benchmark/benchmark.h>

namespace {

void BM_StackPanelMeasureArrange(benchmark::State& state)
{
    auto stack = NeoWPF::Benchmarks::CreateStackWorkload();
    for (auto _ : state) {
        static_cast<void>(_);
        stack->Measure(NeoWPF::Benchmarks::kLayoutWidth,
                       NeoWPF::Benchmarks::kLayoutHeight);
        stack->Arrange(NeoWPF::Benchmarks::kLayoutWidth,
                       NeoWPF::Benchmarks::kLayoutHeight);
        benchmark::DoNotOptimize(stack->GetDesiredHeight());
    }
    state.SetItemsProcessed(state.iterations() * NeoWPF::Benchmarks::kStackChildCount);
}
BENCHMARK(BM_StackPanelMeasureArrange);

}