cmake_minimum_required(VERSION 3.20)

project(NeoWpfXamlTypeDumper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

get_filename_component(NEOWPF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)

set(NEOWPF_PLATFORM "${CMAKE_SYSTEM_NAME}" CACHE STRING "NeoWPF platform archive to link against")
set_property(CACHE NEOWPF_PLATFORM PROPERTY STRINGS Windows Linux)

set(NEOWPF_CONFIG "Debug" CACHE STRING "NeoWPF build config to link against")
set_property(CACHE NEOWPF_CONFIG PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)

if(NEOWPF_PLATFORM STREQUAL "Windows")
    set(NEOWPF_LIBRARY_PATH "${NEOWPF_ROOT}/NWpfBuild/Windows/${NEOWPF_CONFIG}/NeoWPF.lib")
elseif(NEOWPF_PLATFORM STREQUAL "Linux")
    set(NEOWPF_LIBRARY_PATH "${NEOWPF_ROOT}/NWpfBuild/Linux/${NEOWPF_CONFIG}/libNeoWPF.a")
else()
    message(FATAL_ERROR "Unsupported NEOWPF_PLATFORM value: ${NEOWPF_PLATFORM}. Use Windows or Linux.")
endif()

if(NOT EXISTS "${NEOWPF_LIBRARY_PATH}")
    message(FATAL_ERROR
        "NeoWPF prebuilt library not found for ${NEOWPF_PLATFORM}/${NEOWPF_CONFIG}: ${NEOWPF_LIBRARY_PATH}. "
        "Build NeoWPF first or point NEOWPF_CONFIG to an existing config."
    )
endif()

message(STATUS "Linking NeoWpfXamlTypeDumper against ${NEOWPF_LIBRARY_PATH}")

add_executable(NeoWpfXamlTypeDumper src/XamlTypeDumper.cpp)
target_include_directories(NeoWpfXamlTypeDumper PRIVATE
    "${NEOWPF_ROOT}/include"
)
target_link_libraries(NeoWpfXamlTypeDumper PRIVATE
    "${NEOWPF_LIBRARY_PATH}"
)

add_custom_target(run_xaml_type_dump
    COMMAND NeoWpfXamlTypeDumper --output "${NEOWPF_ROOT}/tools/registered_xaml_types.txt"
    DEPENDS NeoWpfXamlTypeDumper
    VERBATIM
)
