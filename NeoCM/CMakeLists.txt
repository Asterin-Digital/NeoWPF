cmake_minimum_required(VERSION 3.20)

project(NeoCM LANGUAGES CXX)

include(CTest)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../NeoWPF/NeoWPFVersion.txt" NEOWPF_VERSION)
string(STRIP "${NEOWPF_VERSION}" NEOWPF_VERSION)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/NeoCMSources.cmake)

set(NEOWPF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../NeoWPF" CACHE PATH
    "Path to the NeoWPF source tree")

set(NEOWPF_FOUNDATION_TARGET "NeoWPF::Foundation" CACHE STRING
    "NeoWPF Foundation target used when NeoCM is embedded in the NeoWPF build")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(NEOWPF_CONFIGURATION "Release")
else()
    set(NEOWPF_CONFIGURATION "Debug")
endif()


if(WIN32)
    set(NEOWPF_PLATFORM_DIR "Windows")
    set(NEOWPF_LIBRARY_SUFFIX ".lib")
    set(NEOWPF_LIBRARY_PREFIX "NeoWPF.Windows.${NEOWPF_CONFIGURATION}.${NEOWPF_VERSION}")
    set(NEOWPF_LIBRARY_CANDIDATES
        "${NEOWPF_ROOT}/NWpfBuild/Windows/${NEOWPF_CONFIGURATION}/${NEOWPF_LIBRARY_PREFIX}${NEOWPF_LIBRARY_SUFFIX}")
else()
    set(NEOWPF_PLATFORM_DIR "Linux")
    set(NEOWPF_LIBRARY_SUFFIX ".a")
    set(NEOWPF_LIBRARY_PREFIX "NeoWPF.Linux.${NEOWPF_CONFIGURATION}.${NEOWPF_VERSION}")
    set(NEOWPF_LIBRARY_CANDIDATES
        "${NEOWPF_ROOT}/NWpfBuild/Linux/${NEOWPF_CONFIGURATION}/${NEOWPF_LIBRARY_PREFIX}${NEOWPF_LIBRARY_SUFFIX}")
endif()

set(NEOWPF_DEFAULT_LIBRARY "")
foreach(_neowpf_candidate IN LISTS NEOWPF_LIBRARY_CANDIDATES)
    if(EXISTS "${_neowpf_candidate}")
        set(NEOWPF_DEFAULT_LIBRARY "${_neowpf_candidate}")
        break()
    endif()
endforeach()

if(NOT NEOWPF_DEFAULT_LIBRARY)
    message(FATAL_ERROR
        "Could not locate a NeoWPF library under ${NEOWPF_ROOT}/NWpfBuild/${NEOWPF_PLATFORM_DIR}/${NEOWPF_CONFIGURATION}. "
        "Expected a versioned artifact like NeoWPF.${NEOWPF_PLATFORM_DIR}.${NEOWPF_CONFIGURATION}.${NEOWPF_VERSION}.lib/a")
endif()

set(NEOWPF_LIBRARY "${NEOWPF_DEFAULT_LIBRARY}" CACHE FILEPATH
    "Path to the versioned NeoWPF library used by the NeoCM compiler")

if(NOT EXISTS "${NEOWPF_ROOT}/include")
    message(FATAL_ERROR
        "NEOWPF_ROOT does not contain an include directory: ${NEOWPF_ROOT}")
endif()

if(NOT TARGET "${NEOWPF_FOUNDATION_TARGET}" AND NOT NEOWPF_LIBRARY)
    message(FATAL_ERROR
        "NeoCM requires NeoWPF.Foundation or a prebuilt NeoWPF library. "
        "Set NEOWPF_FOUNDATION_TARGET or NEOWPF_LIBRARY.")
endif()

if(NOT TARGET "${NEOWPF_FOUNDATION_TARGET}" AND NOT EXISTS "${NEOWPF_LIBRARY}")
    message(FATAL_ERROR
        "NEOWPF_LIBRARY does not exist: ${NEOWPF_LIBRARY}")
endif()

message(STATUS
    "NeoCM: NeoWPF include directory: ${NEOWPF_ROOT}/include")

if(TARGET "${NEOWPF_FOUNDATION_TARGET}")
    message(STATUS
        "NeoCM: using in-tree NeoWPF Foundation target: ${NEOWPF_FOUNDATION_TARGET}")
else()
    message(STATUS
        "NeoCM: using NeoWPF library: ${NEOWPF_LIBRARY}")
endif()

add_library(NeoCM STATIC
    ${NEOCM_UTILS_SOURCES}
    ${NEOCM_COMPILER_SOURCES}
    ${NEOCM_SOURCES}
)

target_include_directories(
    NeoCM
    PRIVATE
        ${NEOCM_INCLUDE_DIRS}
        "${NEOWPF_ROOT}/include"
)

if(TARGET "${NEOWPF_FOUNDATION_TARGET}")
    target_link_libraries(NeoCM PRIVATE "${NEOWPF_FOUNDATION_TARGET}")
elseif(UNIX)
    target_link_libraries(
        NeoCM
        PRIVATE
            "-Wl,--start-group"
            "${NEOWPF_LIBRARY}"
            "-Wl,--end-group"
    )
else()
    target_link_libraries(NeoCM PRIVATE "${NEOWPF_LIBRARY}")
endif()

if(BUILD_TESTING)
    add_executable(NeoCM.GenerateComponent
        tests/GenerateComponent.cpp
    )

    target_include_directories(
        NeoCM.GenerateComponent
        PRIVATE
            ${NEOCM_INCLUDE_DIRS}
            "${NEOWPF_ROOT}/include"
    )

    target_link_libraries(
        NeoCM.GenerateComponent
        PRIVATE
            NeoCM
    )

    target_compile_definitions(
        NeoCM.GenerateComponent
        PRIVATE
            NEOWPF_ENABLE_RUNTIME_XAML
            NEOWPF_INTERNAL_BUILD
    )
endif()

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    FetchContent_MakeAvailable(googletest)

    add_executable(NeoCM.Tests
        tests/NamlArtifactTests.cpp
        tests/NamlCompilerTests.cpp
        tests/GeneratedComponentCompileTests.cpp
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/App.xaml.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/App.xaml.g.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/MainWindow.xaml.g.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/RootWindow.xaml.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/RootWindow.xaml.g.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/ResourceWindow.xaml.g.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/StyledWindow.xaml.g.cpp"
    )

    add_custom_command(
        OUTPUT
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/App.xaml.g.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/App.xaml.g.cpp"
        COMMAND NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/App.xaml"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated"
            "NeoCM::Tests"
            "App"
        DEPENDS
            NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/App.xaml"
        VERBATIM
    )

    add_custom_command(
        OUTPUT
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/MainWindow.xaml.g.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/MainWindow.xaml.g.cpp"
        COMMAND NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/MainWindow.xaml"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated"
        DEPENDS
            NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/MainWindow.xaml"
        VERBATIM
    )

    add_custom_command(
        OUTPUT
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/RootWindow.xaml.g.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/RootWindow.xaml.g.cpp"
        COMMAND NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/RootWindow.xaml"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated"
            "NeoCM::Tests"
            "RootWindow"
        DEPENDS
            NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/RootWindow.xaml"
        VERBATIM
    )

    add_custom_command(
        OUTPUT
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/ResourceWindow.xaml.g.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/ResourceWindow.xaml.g.cpp"
        COMMAND NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/ResourceWindow.xaml"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated"
            "NeoCM::Tests"
            "ResourceWindow"
        DEPENDS
            NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/ResourceWindow.xaml"
        VERBATIM
    )

    add_custom_command(
        OUTPUT
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/StyledWindow.xaml.g.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/StyledWindow.xaml.g.cpp"
        COMMAND NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/StyledWindow.xaml"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated"
            "NeoCM::Tests"
            "StyledWindow"
        DEPENDS
            NeoCM.GenerateComponent
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/StyledWindow.xaml"
        VERBATIM
    )

    add_custom_target(NeoCM.GeneratedComponent
        DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/MainWindow.xaml.g.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/MainWindow.xaml.g.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/App.xaml.g.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/App.xaml.g.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/RootWindow.xaml.g.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/RootWindow.xaml.g.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/ResourceWindow.xaml.g.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/ResourceWindow.xaml.g.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/StyledWindow.xaml.g.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated/StyledWindow.xaml.g.cpp"
    )
    add_dependencies(NeoCM.Tests NeoCM.GeneratedComponent)

    target_include_directories(
        NeoCM.Tests
        PRIVATE
            ${NEOCM_INCLUDE_DIRS}
            "${NEOWPF_ROOT}/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/Generated"
    )

    target_link_libraries(
        NeoCM.Tests
        PRIVATE
            NeoCM
            GTest::gtest_main
    )

    target_compile_definitions(
        NeoCM.Tests
        PRIVATE
            NEOWPF_ENABLE_RUNTIME_XAML
            NEOWPF_INTERNAL_BUILD
    )

    include(GoogleTest)
    gtest_discover_tests(NeoCM.Tests)
endif()

install(
    TARGETS NeoCM
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

target_compile_definitions(NeoCM PRIVATE NEOWPF_ENABLE_RUNTIME_XAML NEOWPF_INTERNAL_BUILD)