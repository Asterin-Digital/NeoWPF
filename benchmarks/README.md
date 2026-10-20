# NeoWPF Performance Benchmarks

## Build and Run

**Windows:**

```powershell
cmake -S . -B build/benchmarks -G "Ninja Multi-Config" -DNEOWPF_BUILD_BENCHMARKS=ON
cmake --build build/benchmarks --config Release --target NeoWPF.PerformanceBenchmarks
```

**Linux:**

```bash
cmake -S . -B build/benchmarks -G "Ninja Multi-Config" \
    -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET=x64-linux \
    -DNEOWPF_RENDERER_BACKEND=Vulkan \
    -DNEOWPF_BUILD_BENCHMARKS=ON \
    -DNEOWPF_USE_NEOLINT=OFF
cmake --build build/benchmarks --config Release --target NeoWPF.PerformanceBenchmarks
```

Run `build/benchmarks/benchmarks/Release/NeoWPF.PerformanceBenchmarks.exe` on Windows or `build/benchmarks/benchmarks/Release/NeoWPF.PerformanceBenchmarks` on Linux.

Save results to JSON with `--benchmark_out=benchmark-results.json --benchmark_out_format=json`.

## Workloads

- `BM_GridMeasureArrange`: Measures and arranges a 16×16 star-sized grid.
- `BM_StackPanelMeasureArrange`: Measures and arranges a vertical panel with 256 elements.
- `BM_RenderCommandsBuild`: Rebuilds the draw-command list for the panel.
- `BM_HitTestMiss`: Performs a hit-test miss through the arranged panel tree.
- `BM_TextBlockMeasure/80` and `/400`: Measures wrapped TextBlocks with 80 or 400 repetitions of a representative UI text phrase.
- `BM_DependencyPropertyGetLocalValue` and `BM_DependencyPropertySetLocalValue`: Measures typed reads and changing writes to a registered local integer property.

Diagnostics logging is disabled during benchmarks. CI runs benchmarks on Linux and uploads JSON results as an artifact.