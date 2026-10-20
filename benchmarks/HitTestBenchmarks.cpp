#include "BenchmarkSupport.h"

#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>

#include <benchmark/benchmark.h>

namespace {

void BM_HitTestMiss(benchmark::State& state)
{
    auto stack = NeoWPF::Benchmarks::CreateStackWorkload();
    auto& inputManager = NeoWPF::Input::InputManager::GetInstance();
    inputManager.SetRootElement(stack.get());
    for (auto _ : state) {
        static_cast<void>(_);
        auto* result = inputManager.HitTest(
            static_cast<int>(NeoWPF::Benchmarks::kLayoutWidth - 1.0),
            static_cast<int>(NeoWPF::Benchmarks::kLayoutHeight - 1.0));
        benchmark::DoNotOptimize(result);
    }
    inputManager.SetRootElement(nullptr);
    state.SetItemsProcessed(state.iterations() * NeoWPF::Benchmarks::kStackChildCount);
}
BENCHMARK(BM_HitTestMiss);

}