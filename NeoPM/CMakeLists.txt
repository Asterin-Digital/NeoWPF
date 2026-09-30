cmake_minimum_required(VERSION 3.20)

project(NeoPM LANGUAGES CXX)

include(CTest)
include(FetchContent)

option(NEOPM_USE_LOCAL_WINUX "Use the local sibling Winux project" ON)

set(NEOPM_WINUX_GIT_TAG "main" CACHE STRING
    "Winux Git tag, branch, or commit to fetch")
set(WINUX_BUILD_TESTS OFF CACHE BOOL "Build Winux tests" FORCE)
if(NEOPM_USE_LOCAL_WINUX)
    set(NEOPM_LOCAL_WINUX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../Winux")
    if(NOT EXISTS "${NEOPM_LOCAL_WINUX_ROOT}")
        message(FATAL_ERROR
            "Could not find local version of Winux at ${NEOPM_LOCAL_WINUX_ROOT}")
    endif()
    if(NOT EXISTS "${NEOPM_LOCAL_WINUX_ROOT}/include")
        message(FATAL_ERROR
            "Could not find local Winux include directory at ${NEOPM_LOCAL_WINUX_ROOT}/include")
    endif()
    add_subdirectory("${NEOPM_LOCAL_WINUX_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/Winux")
else()
    FetchContent_Declare(
        Winux
        GIT_REPOSITORY https://github.com/Past-Horizon/Winux.git
        GIT_TAG ${NEOPM_WINUX_GIT_TAG}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Winux)
endif()

if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING
        "vcpkg triplet used for Windows dependencies")
elseif(UNIX)
    set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING
        "vcpkg triplet used for Linux dependencies")
endif()

set(NEOPM_VCPKG_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
if(NOT EXISTS "${NEOPM_VCPKG_PREFIX}/include/curl/curl.h" OR
   NOT EXISTS "${NEOPM_VCPKG_PREFIX}/share/curl/CURLConfig.cmake" OR
    NOT EXISTS "${NEOPM_VCPKG_PREFIX}/share/nlohmann_json/nlohmann_jsonConfig.cmake" OR
    NOT EXISTS "${NEOPM_VCPKG_PREFIX}/include/zip.h")
    message(FATAL_ERROR
        "Could not locate the vcpkg dependencies for triplet '${VCPKG_TARGET_TRIPLET}' "
        "under ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed. Install the manifest "
        "dependencies with this triplet before configuring NeoPM.")
endif()
list(PREPEND CMAKE_PREFIX_PATH "${NEOPM_VCPKG_PREFIX}")

set(NEOWPF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH
    "Path to the NeoWPF source tree")
include("${NEOWPF_ROOT}/cmake/NeoWPFVersion.cmake")
neowpf_read_artifact_version("${NEOWPF_ROOT}" NEOWPF_VERSION)

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
        "Expected a versioned artifact NeoWPF.${NEOWPF_PLATFORM_DIR}.${NEOWPF_CONFIGURATION}.${NEOWPF_VERSION}.lib/a")
endif()

set(NEOWPF_LIBRARY "${NEOWPF_DEFAULT_LIBRARY}" CACHE FILEPATH
    "Path to the NeoWPF library")

if(NOT EXISTS "${NEOWPF_ROOT}/include")
    message(FATAL_ERROR "NEOWPF_ROOT does not contain an include directory: ${NEOWPF_ROOT}")
endif()

set(NEOCM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../NeoCM" CACHE PATH
    "Path to the NeoCM source tree")
set(NEOPM_PARENT_BUILD_TESTING "${BUILD_TESTING}")
set(BUILD_TESTING OFF)
add_subdirectory("${NEOCM_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/NeoCM")
set(BUILD_TESTING "${NEOPM_PARENT_BUILD_TESTING}")
unset(NEOPM_PARENT_BUILD_TESTING)

if(WIN32)
    set(ZLIB_ROOT "${NEOPM_VCPKG_PREFIX}")
    set(ZLIB_USE_STATIC_LIBS ON)
endif()

find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS algorithm dll filesystem interprocess nowide process)
if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
endif()

add_library(NeoWPF::NeoWPF STATIC IMPORTED GLOBAL)
set_target_properties(NeoWPF::NeoWPF PROPERTIES
    IMPORTED_LOCATION "${NEOWPF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${NEOWPF_ROOT}/include"
)

if(UNIX AND NOT APPLE)
    find_package(Vulkan REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
    pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
    set_target_properties(NeoWPF::NeoWPF PROPERTIES
        INTERFACE_LINK_LIBRARIES "Vulkan::Vulkan;PkgConfig::WAYLAND;PkgConfig::XKBCOMMON"
    )
endif()

set(NEOPM_ARCHIVE_LINK_LIBS
    Winux
    NeoCM
    NeoWPF::NeoWPF
)
if(UNIX)
    list(PREPEND NEOPM_ARCHIVE_LINK_LIBS -Wl,--start-group)
    list(APPEND NEOPM_ARCHIVE_LINK_LIBS -Wl,--end-group)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/NeoPMSources.cmake)

add_executable(NeoPM
    ${NEOPM_SOURCES}
    ${NEOPM_HEADERS}
)

set_target_properties(NeoPM PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/out/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/out/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/out/bin/RelWithDebInfo"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/out/bin/MinSizeRel"
)

target_include_directories(
    NeoPM
    PRIVATE
        ${NEOPM_INCLUDE_DIRS}
        "${NEOCM_ROOT}/include"
        "${NEOWPF_ROOT}/include"
)

target_link_libraries(
    NeoPM
    PRIVATE
        CURL::libcurl
        nlohmann_json::nlohmann_json
        libzip::zip
        Boost::algorithm
        Boost::dll
        Boost::filesystem
        Boost::interprocess
        Boost::nowide
        Boost::process
        ${NEOPM_ARCHIVE_LINK_LIBS}
)

if(WIN32)
    target_link_libraries(NeoPM PRIVATE advapi32)
endif()

    target_compile_definitions(
        NeoPM
        PRIVATE
        _WIN32_WINNT=0x0601
        NEOWPF_ENABLE_RUNTIME_XAML
        NEOWPF_INTERNAL_BUILD
    )

if(BUILD_TESTING)
    add_executable(NeoPM.Tests
        tests/ProjectTypeTests.cpp
        tests/XamlCodegenTests.cpp
    )

    target_link_libraries(
        NeoPM.Tests
        PRIVATE
            GTest::gtest_main
            Boost::filesystem
            ${NEOPM_ARCHIVE_LINK_LIBS}
    )

    target_include_directories(
        NeoPM.Tests
        PRIVATE
            ${NEOPM_INCLUDE_DIRS}
            "${NEOCM_ROOT}/include"
            "${NEOWPF_ROOT}/include"
    )

    target_sources(
        NeoPM.Tests
        PRIVATE
            src/Core/Project/VcxprojBuilder.cpp
            src/Core/Project/CMakeListsBuilder.cpp
            src/Resources/ResourceManifest.cpp
                src/Resources/XamlCodegen.cpp
            src/Utils/FileSystem.cpp
            src/Utils/Guid.cpp
            src/Utils/Logger.cpp
    )

            target_compile_definitions(
            NeoPM.Tests
            PRIVATE
                NEOWPF_ENABLE_RUNTIME_XAML
                NEOWPF_INTERNAL_BUILD
            )

    include(GoogleTest)
    gtest_discover_tests(NeoPM.Tests)
endif()

install(
    TARGETS NeoPM
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)