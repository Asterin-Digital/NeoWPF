#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <benchmark/benchmark.h>

namespace {

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