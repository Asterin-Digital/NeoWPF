cmake_minimum_required(VERSION 3.20)

project(NeoWPFGeneratedNativeCompile LANGUAGES CXX)

set(NEOWPF_GENERATED_DIR "" CACHE PATH "Directory containing generated bindings")
set(NEOWPF_PUBLIC_INCLUDE_DIR "" CACHE PATH "NeoWPF public include directory")

if(NOT EXISTS "${NEOWPF_GENERATED_DIR}/RequestFactory.g.cpp")
    message(FATAL_ERROR "The generated native binding source is missing.")
endif()

if(NOT IS_DIRECTORY "${NEOWPF_PUBLIC_INCLUDE_DIR}/NeoWPF/DotNet")
    message(FATAL_ERROR "The NeoWPF DotNet public headers are missing.")
endif()

add_library(GeneratedNativeBinding OBJECT
    "${NEOWPF_GENERATED_DIR}/RequestFactory.g.cpp"
)
target_include_directories(GeneratedNativeBinding PRIVATE
    "${NEOWPF_GENERATED_DIR}"
    "${NEOWPF_PUBLIC_INCLUDE_DIR}"
)
target_compile_features(GeneratedNativeBinding PRIVATE cxx_std_20)