cmake_minimum_required(VERSION 3.20)

project(NeoWPF LANGUAGES CXX)

include(CTest)

if(WIN32)
    set(NEOWPF_PLATFORM "Windows")
    set(NEOWPF_DEFAULT_RENDERER_BACKEND "D3D11")
elseif(UNIX)
    set(NEOWPF_PLATFORM "Linux")
    set(NEOWPF_DEFAULT_RENDERER_BACKEND "Vulkan")
else()
    message(FATAL_ERROR "Unsupported NeoWPF platform: ${CMAKE_SYSTEM_NAME}")
endif()

set(NEOWPF_RENDERER_BACKEND "${NEOWPF_DEFAULT_RENDERER_BACKEND}" CACHE STRING
    "Renderer backend to compile (D3D11 or Vulkan)")
set_property(CACHE NEOWPF_RENDERER_BACKEND PROPERTY STRINGS D3D11 Vulkan)
string(TOLOWER "${NEOWPF_RENDERER_BACKEND}" NEOWPF_RENDERER_BACKEND_LOWER)
if(NOT NEOWPF_RENDERER_BACKEND_LOWER STREQUAL "d3d11" AND
   NOT NEOWPF_RENDERER_BACKEND_LOWER STREQUAL "vulkan")
    message(FATAL_ERROR
        "Unsupported NEOWPF_RENDERER_BACKEND='${NEOWPF_RENDERER_BACKEND}'. "
        "Choose D3D11 or Vulkan.")
endif()
if(NOT WIN32 AND NEOWPF_RENDERER_BACKEND_LOWER STREQUAL "d3d11")
    message(FATAL_ERROR "D3D11 is only supported on Windows.")
endif()

if(NEOWPF_RENDERER_BACKEND_LOWER STREQUAL "d3d11")
    set(NEOWPF_RENDERER_BACKEND "D3D11")
else()
    set(NEOWPF_RENDERER_BACKEND "Vulkan")
    find_package(Vulkan REQUIRED)
endif()

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
    pkg_check_modules(FREETYPE REQUIRED IMPORTED_TARGET freetype2)
    pkg_check_modules(HARFBUZZ REQUIRED IMPORTED_TARGET harfbuzz)
    find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
    pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
    set(NEOWPF_XDG_SHELL_XML "${WAYLAND_PROTOCOLS_DATADIR}/stable/xdg-shell/xdg-shell.xml")
    if(NOT EXISTS "${NEOWPF_XDG_SHELL_XML}")
        message(FATAL_ERROR "Could not find xdg-shell protocol XML: ${NEOWPF_XDG_SHELL_XML}")
    endif()
    set(NEOWPF_WAYLAND_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/wayland")
    set(NEOWPF_XDG_SHELL_HEADER "${NEOWPF_WAYLAND_GENERATED_DIR}/xdg-shell-client-protocol.h")
    set(NEOWPF_XDG_SHELL_SOURCE "${NEOWPF_WAYLAND_GENERATED_DIR}/xdg-shell-protocol.cpp")
    add_custom_command(
        OUTPUT "${NEOWPF_XDG_SHELL_HEADER}" "${NEOWPF_XDG_SHELL_SOURCE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_WAYLAND_GENERATED_DIR}"
        COMMAND "${WAYLAND_SCANNER}" client-header "${NEOWPF_XDG_SHELL_XML}" "${NEOWPF_XDG_SHELL_HEADER}"
        COMMAND "${WAYLAND_SCANNER}" private-code "${NEOWPF_XDG_SHELL_XML}" "${NEOWPF_XDG_SHELL_SOURCE}"
        DEPENDS "${NEOWPF_XDG_SHELL_XML}"
        VERBATIM
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties("${NEOWPF_XDG_SHELL_SOURCE}"
            PROPERTIES COMPILE_OPTIONS "-Wno-attributes")
    endif()
endif()

if(WIN32)
    find_program(NEOWPF_WHERE_EXECUTABLE where REQUIRED)
    execute_process(
        COMMAND "${NEOWPF_WHERE_EXECUTABLE}" vcpkg
        OUTPUT_VARIABLE NEOWPF_VCPKG_EXECUTABLES
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE NEOWPF_WHERE_RESULT
    )
    if(NEOWPF_WHERE_RESULT EQUAL 0)
        string(REPLACE "\r\n" ";" NEOWPF_VCPKG_EXECUTABLES "${NEOWPF_VCPKG_EXECUTABLES}")
        string(REPLACE "\n" ";" NEOWPF_VCPKG_EXECUTABLES "${NEOWPF_VCPKG_EXECUTABLES}")
        foreach(NEOWPF_VCPKG_EXECUTABLE IN LISTS NEOWPF_VCPKG_EXECUTABLES)
            get_filename_component(NEOWPF_VCPKG_CANDIDATE_ROOT "${NEOWPF_VCPKG_EXECUTABLE}" DIRECTORY)
            foreach(NEOWPF_VCPKG_CANDIDATE_TRIPLET IN ITEMS
                "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows-static-md"
                "${NEOWPF_VCPKG_CANDIDATE_ROOT}/installed/x64-windows-static-md")
                if(EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/include/ft2build.h" AND
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/include/harfbuzz/hb.h" AND
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/share/freetype/freetype-config.cmake" AND
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/share/harfbuzz/harfbuzzConfig.cmake")
                set(NEOWPF_VCPKG_ROOT "${NEOWPF_VCPKG_CANDIDATE_ROOT}")
                set(NEOWPF_VCPKG_TRIPLET "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}")
                get_filename_component(VCPKG_INSTALLED_DIR "${NEOWPF_VCPKG_TRIPLET}" DIRECTORY)
                set(_VCPKG_INSTALLED_DIR "${VCPKG_INSTALLED_DIR}")
                set(VCPKG_TARGET_TRIPLET "x64-windows-static-md")
                list(PREPEND CMAKE_PREFIX_PATH "${NEOWPF_VCPKG_TRIPLET}")
                break()
                endif()
            endforeach()
            if(NEOWPF_VCPKG_TRIPLET)
                break()
            endif()
        endforeach()
    endif()
    if(NOT NEOWPF_VCPKG_TRIPLET)
        message(FATAL_ERROR
            "Could not locate a vcpkg installation containing the x64-windows-static-md "
            "FreeType and HarfBuzz packages. Ensure vcpkg is available on PATH.")
    endif()
    find_package(Freetype CONFIG REQUIRED)
    find_package(harfbuzz CONFIG REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/NeoWPFSources.cmake)
if(UNIX)
    list(APPEND NEOWPF_SOURCES "${NEOWPF_XDG_SHELL_SOURCE}")
    list(APPEND NEOWPF_HEADERS "${NEOWPF_XDG_SHELL_HEADER}")
endif()

add_library(NeoWPF STATIC ${NEOWPF_SOURCES} ${NEOWPF_HEADERS})
add_library(NeoWPF::NeoWPF ALIAS NeoWPF)

if(WIN32)
    set(NEOWPF_HARFBUZZ_DEBUG_ARCHIVE "${NEOWPF_VCPKG_TRIPLET}/debug/lib/harfbuzz.lib")
    set(NEOWPF_FREETYPE_DEBUG_ARCHIVE "${NEOWPF_VCPKG_TRIPLET}/debug/lib/freetyped.lib")
    set(NEOWPF_HARFBUZZ_RELEASE_ARCHIVE "${NEOWPF_VCPKG_TRIPLET}/lib/harfbuzz.lib")
    set(NEOWPF_FREETYPE_RELEASE_ARCHIVE "${NEOWPF_VCPKG_TRIPLET}/lib/freetype.lib")
    foreach(NEOWPF_TEXT_ARCHIVE IN ITEMS
        "${NEOWPF_HARFBUZZ_DEBUG_ARCHIVE}"
        "${NEOWPF_FREETYPE_DEBUG_ARCHIVE}"
        "${NEOWPF_HARFBUZZ_RELEASE_ARCHIVE}"
        "${NEOWPF_FREETYPE_RELEASE_ARCHIVE}")
        if(NOT EXISTS "${NEOWPF_TEXT_ARCHIVE}")
            message(FATAL_ERROR "Missing static text archive: ${NEOWPF_TEXT_ARCHIVE}")
        endif()
    endforeach()

    add_custom_command(TARGET NeoWPF POST_BUILD
        COMMAND "${CMAKE_AR}"
            "/OUT:$<TARGET_FILE:NeoWPF>.merged"
            "$<TARGET_FILE:NeoWPF>"
            "$<IF:$<CONFIG:Debug>,${NEOWPF_HARFBUZZ_DEBUG_ARCHIVE},${NEOWPF_HARFBUZZ_RELEASE_ARCHIVE}>"
            "$<IF:$<CONFIG:Debug>,${NEOWPF_FREETYPE_DEBUG_ARCHIVE},${NEOWPF_FREETYPE_RELEASE_ARCHIVE}>"
        COMMAND "${CMAKE_COMMAND}" -E copy
            "$<TARGET_FILE:NeoWPF>.merged"
            "$<TARGET_FILE:NeoWPF>"
        COMMAND "${CMAKE_COMMAND}" -E rm -f "$<TARGET_FILE:NeoWPF>.merged"
        VERBATIM
    )
endif()

set_target_properties(NeoWPF PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/x64"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/x64/Debug"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/x64/Release"
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/x64/RelWithDebInfo"
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/x64/MinSizeRel"
)

target_include_directories(NeoWPF
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
if(WIN32)
    target_include_directories(NeoWPF PRIVATE "${NEOWPF_VCPKG_TRIPLET}/include")
    target_include_directories(NeoWPF PRIVATE "${NEOWPF_VCPKG_TRIPLET}/include/harfbuzz")
endif()
if(UNIX)
    target_include_directories(NeoWPF PRIVATE "${NEOWPF_WAYLAND_GENERATED_DIR}")
endif()

target_compile_definitions(NeoWPF
    PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

if(WIN32)
    target_compile_definitions(NeoWPF PRIVATE
        _CONSOLE
        UNICODE
        _UNICODE
    )
endif()

if(MSVC)
    target_compile_options(NeoWPF PRIVATE
        /W3
        /sdl
        /permissive-
        /Zc:wchar_t
        /Zc:forScope
        /Zc:inline
        /EHsc
    )

    target_compile_options(NeoWPF PRIVATE
        $<$<CONFIG:Release>:/Gy>
        $<$<CONFIG:Release>:/Oi>
        $<$<CONFIG:Release>:/GL>
    )

    set_property(TARGET NeoWPF PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
endif()

if(WIN32)
    target_link_libraries(NeoWPF
        PRIVATE
            d3d11
            dxgi
            dbghelp
            winhttp
            ole32
            windowscodecs
    )
else()
    target_link_libraries(NeoWPF
        PUBLIC
            Vulkan::Vulkan
            PkgConfig::WAYLAND
            PkgConfig::FREETYPE
            PkgConfig::HARFBUZZ
    )
    message(STATUS "NeoWPF Linux configuration selected with Vulkan renderer backend.")
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${NEOWPF_SOURCES} ${NEOWPF_HEADERS})

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

    add_executable(NeoWPF.AutomatedTests
        tests/Automated/ResourceKeyTests.cpp
        tests/Automated/EventTests.cpp
        tests/Automated/DependencyPropertyTests.cpp
    )

    target_link_libraries(NeoWPF.AutomatedTests
        PRIVATE
            NeoWPF::NeoWPF
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(NeoWPF.AutomatedTests)
endif()

install(TARGETS NeoWPF ARCHIVE DESTINATION lib)
install(DIRECTORY include/NeoWPF DESTINATION include)
install(FILES LICENSE DESTINATION .)
