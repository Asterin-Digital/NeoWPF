#include <NeoWPF/Diagnostics/Logger.h>
#include "BenchmarkSupport.h"

#include <benchmark/benchmark.h>

#include <cstdio>

int main(int argc, char** argv)
{
    NeoWPF::Diagnostics::SetGlobalLevel(NeoWPF::Diagnostics::Level::Off);
    if (!NeoWPF::Benchmarks::RegisterBenchmarkFont()) {
        std::fputs("Failed to register NeoWPF's embedded benchmark font.\n", stderr);
        return 1;
    }
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();
    return 0;
}