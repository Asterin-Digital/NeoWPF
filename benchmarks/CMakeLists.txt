set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(NeoWPF.PerformanceBenchmarks
    Main.cpp
)

target_link_libraries(NeoWPF.PerformanceBenchmarks
    PRIVATE
        NeoWPF::NeoWPF
    benchmark::benchmark
)

target_compile_features(NeoWPF.PerformanceBenchmarks PRIVATE cxx_std_20)
target_compile_definitions(NeoWPF.PerformanceBenchmarks PRIVATE NEOWPF_INTERNAL_BUILD)