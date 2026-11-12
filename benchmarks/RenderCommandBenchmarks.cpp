#include "BenchmarkSupport.h"

#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>

#include <benchmark/benchmark.h>

namespace {

void BM_RenderCommandsBuild(benchmark::State& state)
{
    auto stack = NeoWPF::Benchmarks::CreateStackWorkload();
    NeoWPF::Rendering::CommandList commandList;
    for (auto _ : state) {
        static_cast<void>(_);
        commandList.Clear();
        stack->BuildRenderCommands(commandList);
        benchmark::DoNotOptimize(commandList.commands.data());
        benchmark::ClobberMemory();
    }
    const auto commandCount = commandList.commands.size();
    state.counters["CommandCount"] = static_cast<double>(commandCount);
    state.counters["CommandSizeBytes"] = sizeof(NeoWPF::Rendering::CommandList::Command);
    state.counters["CommandStorageBytes"] = static_cast<double>(
        commandCount * sizeof(NeoWPF::Rendering::CommandList::Command));
    state.SetItemsProcessed(state.iterations() * NeoWPF::Benchmarks::kStackChildCount);
}
BENCHMARK(BM_RenderCommandsBuild);

}