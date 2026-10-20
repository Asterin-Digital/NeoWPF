#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Input/InputManager.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace {

constexpr int kGridTrackCount = 16;
constexpr int kGridChildCount = kGridTrackCount * kGridTrackCount;
constexpr std::size_t kStackChildCount = 256;
constexpr double kElementExtent = 8.0;
constexpr double kLayoutWidth = 1024.0;
constexpr double kLayoutHeight = 4096.0;
constexpr double kTextWidth = 560.0;
constexpr double kTextHeight = 240.0;

class BenchmarkDependencyObject final : public NeoWPF::UI::Properties::DependencyObject {
};

const NeoWPF::UI::Properties::DependencyProperty& BenchmarkIntProperty()
{
    using NeoWPF::UI::Properties::DependencyProperty;
    using NeoWPF::UI::Properties::PropertyMetadata;
    static const auto& property = DependencyProperty::Register<BenchmarkDependencyObject, int>(
        "Benchmarks.IntValue", PropertyMetadata{0});
    return property;
}

std::wstring CreateTextWorkload(std::size_t wordCount)
{
    constexpr std::wstring_view phrase = L"Retained UI text layout benchmark. ";
    std::wstring text;
    text.reserve(wordCount * phrase.size());
    for (std::size_t index = 0; index < wordCount; ++index) {
        text.append(phrase);
    }
    return text;
}

class BenchmarkElement final : public NeoWPF::UI::FrameworkElement {
public:
    void Measure(double, double) override
    {
        SetDesiredSize(kElementExtent, kElementExtent);
    }

    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override
    {
        commands.DrawRect(
            GetLayoutOffsetX(),
            GetLayoutOffsetY(),
            GetFinalWidth(),
            GetFinalHeight(),
            0xFF336699u);
    }
};

std::shared_ptr<NeoWPF::UI::Grid> CreateGridWorkload()
{
    auto grid = std::make_shared<NeoWPF::UI::Grid>();
    for (int index = 0; index < kGridTrackCount; ++index) {
        grid->AddRow(NeoWPF::UI::SizeMode::Star, 1);
        grid->AddColumn(NeoWPF::UI::SizeMode::Star, 1);
    }

    for (int index = 0; index < kGridChildCount; ++index) {
        auto child = std::make_shared<BenchmarkElement>();
        grid->AddChild(child, index / kGridTrackCount, index % kGridTrackCount);
    }

    return grid;
}

std::shared_ptr<NeoWPF::UI::StackPanel> CreateStackWorkload()
{
    auto panel = std::make_shared<NeoWPF::UI::StackPanel>();
    for (std::size_t index = 0; index < kStackChildCount; ++index) {
        panel->AddChild(std::make_shared<BenchmarkElement>());
    }

    panel->Measure(kLayoutWidth, kLayoutHeight);
    panel->Arrange(kLayoutWidth, kLayoutHeight);
    return panel;
}

void BM_GridMeasureArrange(benchmark::State& state)
{
    auto grid = CreateGridWorkload();
    for (auto _ : state) {
        static_cast<void>(_);
        grid->Measure(kLayoutWidth, kLayoutHeight);
        grid->Arrange(kLayoutWidth, kLayoutHeight);
        benchmark::DoNotOptimize(grid->GetDesiredWidth());
    }
    state.SetItemsProcessed(state.iterations() * kGridChildCount);
}
BENCHMARK(BM_GridMeasureArrange);

void BM_StackPanelMeasureArrange(benchmark::State& state)
{
    auto stack = CreateStackWorkload();
    for (auto _ : state) {
        static_cast<void>(_);
        stack->Measure(kLayoutWidth, kLayoutHeight);
        stack->Arrange(kLayoutWidth, kLayoutHeight);
        benchmark::DoNotOptimize(stack->GetDesiredHeight());
    }
    state.SetItemsProcessed(state.iterations() * kStackChildCount);
}
BENCHMARK(BM_StackPanelMeasureArrange);

void BM_RenderCommandsBuild(benchmark::State& state)
{
    auto stack = CreateStackWorkload();
    NeoWPF::Rendering::CommandList commandList;
    for (auto _ : state) {
        static_cast<void>(_);
        commandList.Clear();
        stack->BuildRenderCommands(commandList);
        benchmark::DoNotOptimize(commandList.commands.data());
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * kStackChildCount);
}
BENCHMARK(BM_RenderCommandsBuild);

void BM_HitTestMiss(benchmark::State& state)
{
    auto stack = CreateStackWorkload();
    auto& inputManager = NeoWPF::Input::InputManager::GetInstance();
    inputManager.SetRootElement(stack.get());
    for (auto _ : state) {
        static_cast<void>(_);
        auto* result = inputManager.HitTest(
            static_cast<int>(kLayoutWidth - 1.0),
            static_cast<int>(kLayoutHeight - 1.0));
        benchmark::DoNotOptimize(result);
    }
    inputManager.SetRootElement(nullptr);
    state.SetItemsProcessed(state.iterations() * kStackChildCount);
}
BENCHMARK(BM_HitTestMiss);

void BM_TextBlockMeasure(benchmark::State& state)
{
    const auto text = CreateTextWorkload(static_cast<std::size_t>(state.range(0)));
    NeoWPF::UI::TextBlock textBlock(text);
    textBlock.SetTextWrapping(NeoWPF::UI::TextWrapping::Wrap);
    for (auto _ : state) {
        static_cast<void>(_);
        textBlock.Measure(kTextWidth, kTextHeight);
        benchmark::DoNotOptimize(textBlock.GetDesiredWidth());
    }
    state.SetItemsProcessed(state.iterations() * static_cast<std::int64_t>(text.size()));
}
BENCHMARK(BM_TextBlockMeasure)->Arg(8)->Arg(80)->Arg(400);

void BM_DependencyPropertyGetLocalValue(benchmark::State& state)
{
    BenchmarkDependencyObject object;
    const auto& property = BenchmarkIntProperty();
    object.SetValue(property, 42);
    for (auto _ : state) {
        static_cast<void>(_);
        benchmark::DoNotOptimize(object.GetValue<int>(property));
    }
}
BENCHMARK(BM_DependencyPropertyGetLocalValue);

void BM_DependencyPropertySetLocalValue(benchmark::State& state)
{
    BenchmarkDependencyObject object;
    const auto& property = BenchmarkIntProperty();
    object.SetValue(property, 42);
    int value = 43;
    for (auto _ : state) {
        static_cast<void>(_);
        object.SetValue(property, value);
        benchmark::DoNotOptimize(object.GetValue<int>(property));
        value = value == 43 ? 44 : 43;
    }
}
BENCHMARK(BM_DependencyPropertySetLocalValue);

}

int main(int argc, char** argv)
{
    NeoWPF::Diagnostics::SetGlobalLevel(NeoWPF::Diagnostics::Level::Off);
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}