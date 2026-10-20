#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>

#include <benchmark/benchmark.h>

#include <memory>

namespace {

constexpr int kTrackCount = 16;
constexpr int kChildCount = kTrackCount * kTrackCount;
constexpr double kAvailableWidth = 1024.0;
constexpr double kAvailableHeight = 4096.0;
constexpr double kChildExtent = 8.0;

class GridBenchmarkElement final : public NeoWPF::UI::FrameworkElement {
public:
    void Measure(double, double) override
    {
        SetDesiredSize(kChildExtent, kChildExtent);
    }
};

std::shared_ptr<NeoWPF::UI::Grid> CreateGridWorkload()
{
    auto grid = std::make_shared<NeoWPF::UI::Grid>();
    for (int index = 0; index < kTrackCount; ++index) {
        grid->AddRow(NeoWPF::UI::SizeMode::Star, 1);
        grid->AddColumn(NeoWPF::UI::SizeMode::Star, 1);
    }
    for (int index = 0; index < kChildCount; ++index) {
        grid->AddChild(std::make_shared<GridBenchmarkElement>(),
                       index / kTrackCount, index % kTrackCount);
    }
    return grid;
}

void BM_GridMeasureArrange(benchmark::State& state)
{
    auto grid = CreateGridWorkload();
    for (auto _ : state) {
        static_cast<void>(_);
        grid->Measure(kAvailableWidth, kAvailableHeight);
        grid->Arrange(kAvailableWidth, kAvailableHeight);
        benchmark::DoNotOptimize(grid->GetDesiredWidth());
    }
    state.SetItemsProcessed(state.iterations() * kChildCount);
}
BENCHMARK(BM_GridMeasureArrange);

}