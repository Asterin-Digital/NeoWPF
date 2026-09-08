cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/NeoWPFVersion.txt" NEOWPF_VERSION)
string(STRIP "${NEOWPF_VERSION}" NEOWPF_VERSION)
if(NOT NEOWPF_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "NeoWPFVersion.txt must contain a semantic version")
endif()
project(NeoWPF VERSION "${NEOWPF_VERSION}" LANGUAGES CXX)

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
set(NEOWPF_IGNORE_BAD_XAML OFF CACHE BOOL # turn this on when compiling new XAML types so the compiler will not stop builds due to stable build of pre-built NeoWPF library 
    "Warn and omit generated outputs when XAML compilation fails")
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
    find_program(NEOWPF_GLSLANG_VALIDATOR glslangValidator REQUIRED)
endif()

if(UNIX)
    set(NEOWPF_VULKAN_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/Rendering/Vulkan/Shaders")
    set(NEOWPF_VULKAN_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/vulkan")
    set(NEOWPF_SOLID_VERT_SPV "${NEOWPF_VULKAN_GENERATED_DIR}/solid.vert.spv")
    set(NEOWPF_SOLID_FRAG_SPV "${NEOWPF_VULKAN_GENERATED_DIR}/solid.frag.spv")
    set(NEOWPF_SOLID_VERT_HEADER "${NEOWPF_VULKAN_GENERATED_DIR}/solid.vert.spv.h")
    set(NEOWPF_SOLID_FRAG_HEADER "${NEOWPF_VULKAN_GENERATED_DIR}/solid.frag.spv.h")
    set(NEOWPF_TEXTURED_VERT_SPV "${NEOWPF_VULKAN_GENERATED_DIR}/textured.vert.spv")
    set(NEOWPF_TEXTURED_FRAG_SPV "${NEOWPF_VULKAN_GENERATED_DIR}/textured.frag.spv")
    set(NEOWPF_TEXTURED_VERT_HEADER "${NEOWPF_VULKAN_GENERATED_DIR}/textured.vert.spv.h")
    set(NEOWPF_TEXTURED_FRAG_HEADER "${NEOWPF_VULKAN_GENERATED_DIR}/textured.frag.spv.h")
    add_custom_command(
        OUTPUT "${NEOWPF_SOLID_VERT_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_VULKAN_GENERATED_DIR}"
        COMMAND "${NEOWPF_GLSLANG_VALIDATOR}" -V "${NEOWPF_VULKAN_SHADER_DIR}/solid.vert" -o "${NEOWPF_SOLID_VERT_SPV}"
        COMMAND ${CMAKE_COMMAND} -DINPUT=${NEOWPF_SOLID_VERT_SPV} -DOUTPUT=${NEOWPF_SOLID_VERT_HEADER} -DSYMBOL=neowpf_solid_vert_spv -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake"
        DEPENDS "${NEOWPF_VULKAN_SHADER_DIR}/solid.vert" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${NEOWPF_SOLID_FRAG_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_VULKAN_GENERATED_DIR}"
        COMMAND "${NEOWPF_GLSLANG_VALIDATOR}" -V "${NEOWPF_VULKAN_SHADER_DIR}/solid.frag" -o "${NEOWPF_SOLID_FRAG_SPV}"
        COMMAND ${CMAKE_COMMAND} -DINPUT=${NEOWPF_SOLID_FRAG_SPV} -DOUTPUT=${NEOWPF_SOLID_FRAG_HEADER} -DSYMBOL=neowpf_solid_frag_spv -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake"
        DEPENDS "${NEOWPF_VULKAN_SHADER_DIR}/solid.frag" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${NEOWPF_TEXTURED_VERT_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_VULKAN_GENERATED_DIR}"
        COMMAND "${NEOWPF_GLSLANG_VALIDATOR}" -V "${NEOWPF_VULKAN_SHADER_DIR}/textured.vert" -o "${NEOWPF_TEXTURED_VERT_SPV}"
        COMMAND ${CMAKE_COMMAND} -DINPUT=${NEOWPF_TEXTURED_VERT_SPV} -DOUTPUT=${NEOWPF_TEXTURED_VERT_HEADER} -DSYMBOL=neowpf_textured_vert_spv -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake"
        DEPENDS "${NEOWPF_VULKAN_SHADER_DIR}/textured.vert" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${NEOWPF_TEXTURED_FRAG_HEADER}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_VULKAN_GENERATED_DIR}"
        COMMAND "${NEOWPF_GLSLANG_VALIDATOR}" -V "${NEOWPF_VULKAN_SHADER_DIR}/textured.frag" -o "${NEOWPF_TEXTURED_FRAG_SPV}"
        COMMAND ${CMAKE_COMMAND} -DINPUT=${NEOWPF_TEXTURED_FRAG_SPV} -DOUTPUT=${NEOWPF_TEXTURED_FRAG_HEADER} -DSYMBOL=neowpf_textured_frag_spv -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake"
        DEPENDS "${NEOWPF_VULKAN_SHADER_DIR}/textured.frag" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedSpirv.cmake"
        VERBATIM
    )
    set_source_files_properties("${NEOWPF_SOLID_VERT_HEADER}" "${NEOWPF_SOLID_FRAG_HEADER}"
        "${NEOWPF_TEXTURED_VERT_HEADER}" "${NEOWPF_TEXTURED_FRAG_HEADER}"
        PROPERTIES HEADER_FILE_ONLY TRUE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
    pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
    pkg_check_modules(FREETYPE REQUIRED IMPORTED_TARGET freetype2)
    pkg_check_modules(HARFBUZZ REQUIRED IMPORTED_TARGET harfbuzz)
    pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
    find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
    pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
    set(NEOWPF_XDG_SHELL_XML "${WAYLAND_PROTOCOLS_DATADIR}/stable/xdg-shell/xdg-shell.xml")
    if(NOT EXISTS "${NEOWPF_XDG_SHELL_XML}")
        message(FATAL_ERROR "Could not find xdg-shell protocol XML: ${NEOWPF_XDG_SHELL_XML}")
    endif()
    set(NEOWPF_XDG_DECORATION_XML "${WAYLAND_PROTOCOLS_DATADIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml")
    if(NOT EXISTS "${NEOWPF_XDG_DECORATION_XML}")
        set(NEOWPF_XDG_DECORATION_XML "${WAYLAND_PROTOCOLS_DATADIR}/staging/xdg-decoration/xdg-decoration-unstable-v1.xml")
    endif()
    if(NOT EXISTS "${NEOWPF_XDG_DECORATION_XML}")
        message(FATAL_ERROR "Could not find xdg-decoration protocol XML in ${WAYLAND_PROTOCOLS_DATADIR}")
    endif()
    set(NEOWPF_WAYLAND_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/wayland")
    set(NEOWPF_XDG_SHELL_HEADER "${NEOWPF_WAYLAND_GENERATED_DIR}/xdg-shell-client-protocol.h")
    set(NEOWPF_XDG_SHELL_SOURCE "${NEOWPF_WAYLAND_GENERATED_DIR}/xdg-shell-protocol.cpp")
    set(NEOWPF_XDG_DECORATION_HEADER "${NEOWPF_WAYLAND_GENERATED_DIR}/xdg-decoration-client-protocol.h")
    set(NEOWPF_XDG_DECORATION_SOURCE "${NEOWPF_WAYLAND_GENERATED_DIR}/xdg-decoration-protocol.cpp")
    add_custom_command(
        OUTPUT "${NEOWPF_XDG_SHELL_HEADER}" "${NEOWPF_XDG_SHELL_SOURCE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_WAYLAND_GENERATED_DIR}"
        COMMAND "${WAYLAND_SCANNER}" client-header "${NEOWPF_XDG_SHELL_XML}" "${NEOWPF_XDG_SHELL_HEADER}"
        COMMAND "${WAYLAND_SCANNER}" private-code "${NEOWPF_XDG_SHELL_XML}" "${NEOWPF_XDG_SHELL_SOURCE}"
        DEPENDS "${NEOWPF_XDG_SHELL_XML}"
        VERBATIM
    )
    add_custom_command(
        OUTPUT "${NEOWPF_XDG_DECORATION_HEADER}" "${NEOWPF_XDG_DECORATION_SOURCE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_WAYLAND_GENERATED_DIR}"
        COMMAND "${WAYLAND_SCANNER}" client-header "${NEOWPF_XDG_DECORATION_XML}" "${NEOWPF_XDG_DECORATION_HEADER}"
        COMMAND "${WAYLAND_SCANNER}" private-code "${NEOWPF_XDG_DECORATION_XML}" "${NEOWPF_XDG_DECORATION_SOURCE}"
        DEPENDS "${NEOWPF_XDG_DECORATION_XML}"
        VERBATIM
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set_source_files_properties("${NEOWPF_XDG_SHELL_SOURCE}"
            PROPERTIES COMPILE_OPTIONS "-Wno-attributes")
        set_source_files_properties("${NEOWPF_XDG_DECORATION_SOURCE}"
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
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/include/curl/curl.h" AND
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/include/stb_image.h" AND
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/include/zlib.h" AND
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/lib/zs.lib" AND
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/debug/lib/zsd.lib" AND
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/share/freetype/freetype-config.cmake" AND
                   EXISTS "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}/share/harfbuzz/harfbuzzConfig.cmake")
                    set(NEOWPF_VCPKG_ROOT "${NEOWPF_VCPKG_CANDIDATE_ROOT}")
                    set(NEOWPF_VCPKG_TRIPLET "${NEOWPF_VCPKG_CANDIDATE_TRIPLET}")
                    get_filename_component(VCPKG_INSTALLED_DIR "${NEOWPF_VCPKG_TRIPLET}" DIRECTORY)
                    set(_VCPKG_INSTALLED_DIR "${VCPKG_INSTALLED_DIR}")
                    set(VCPKG_TARGET_TRIPLET "x64-windows-static-md")
                    list(PREPEND CMAKE_PREFIX_PATH "${NEOWPF_VCPKG_TRIPLET}")
                    list(PREPEND CMAKE_MODULE_PATH "${NEOWPF_VCPKG_TRIPLET}/share/stb")
                    set(ZLIB_ROOT "${NEOWPF_VCPKG_TRIPLET}")
                    set(ZLIB_USE_STATIC_LIBS ON)
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
            "FreeType, HarfBuzz, and Zlib packages. Ensure vcpkg is available on PATH "
            "and install the missing CMake dependency.")
    endif()
    set(ZLIB_INCLUDE_DIR "${NEOWPF_VCPKG_TRIPLET}/include" CACHE PATH "Zlib include directory" FORCE)
    set(ZLIB_LIBRARY_RELEASE "${NEOWPF_VCPKG_TRIPLET}/lib/zs.lib" CACHE FILEPATH "Zlib release library" FORCE)
    set(ZLIB_LIBRARY_DEBUG "${NEOWPF_VCPKG_TRIPLET}/debug/lib/zsd.lib" CACHE FILEPATH "Zlib debug library" FORCE)
    find_package(Freetype CONFIG REQUIRED)
    find_package(harfbuzz CONFIG REQUIRED)
endif()

if(UNIX)
    set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING
        "vcpkg triplet used for Linux dependencies")
    set(VCPKG_LIBRARY_LINKAGE "static" CACHE STRING
        "Build Linux vcpkg libraries as static libraries")
    set(NEOWPF_VCPKG_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
    if(NOT EXISTS "${NEOWPF_VCPKG_PREFIX}/include/curl/curl.h" OR
       NOT EXISTS "${NEOWPF_VCPKG_PREFIX}/include/stb_image.h" OR
       NOT EXISTS "${NEOWPF_VCPKG_PREFIX}/share/curl/CURLConfig.cmake" OR
       NOT EXISTS "${NEOWPF_VCPKG_PREFIX}/share/stb/FindStb.cmake" OR
       NOT EXISTS "${NEOWPF_VCPKG_PREFIX}/share/freetype/freetype-config.cmake")
        message(FATAL_ERROR
            "Could not locate the static Linux vcpkg triplet '${VCPKG_TARGET_TRIPLET}' "
            "under ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed. Install the manifest "
            "dependencies with this triplet before configuring NeoWPF.")
    endif()
    list(PREPEND CMAKE_PREFIX_PATH "${NEOWPF_VCPKG_PREFIX}")
    list(PREPEND CMAKE_MODULE_PATH "${NEOWPF_VCPKG_PREFIX}/share/stb")
endif()

find_package(ZLIB REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(Stb REQUIRED)
if(UNIX)
    find_package(MbedTLS CONFIG REQUIRED)
    find_package(OpenSSL CONFIG REQUIRED)
    find_package(Freetype CONFIG REQUIRED)
    find_package(PNG REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NEOWPF_STATIC_DEPENDENCIES
    "Merge static NeoWPF dependencies into the NeoWPF archive"
    ON) 
option(APPLY_CONTROL_STYLES
    "Apply generated XAML styles from control constructors"
    ON)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/XamlCompiler${CMAKE_EXECUTABLE_SUFFIX}")
    set(NEOWPF_XAML_COMPILER
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/XamlCompiler${CMAKE_EXECUTABLE_SUFFIX}")
else()
    set(NEOWPF_XAML_COMPILER
        "${CMAKE_CURRENT_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/$<CONFIG>/tools/XamlCompiler${CMAKE_EXECUTABLE_SUFFIX}")
endif()

include(cmake/NeoWPFSources.cmake)
add_library(NeoNaml OBJECT
    src/NeoNaml/NamlArtifact.cpp
)
add_library(NeoNaml::NeoNaml ALIAS NeoNaml)
target_include_directories(NeoNaml
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

include(cmake/NeoWPFXamlCommands.cmake)

add_library(NeoWPF.Foundation OBJECT ${NEOWPF_SOURCES} ${NEOWPF_HEADERS})
add_library(NeoWPF::Foundation ALIAS NeoWPF.Foundation
)
target_sources(NeoWPF.Foundation PRIVATE $<TARGET_OBJECTS:NeoNaml>)
target_include_directories(NeoWPF.Foundation
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_BINARY_DIR}/generated/xaml"
)
target_compile_definitions(NeoWPF.Foundation
    PUBLIC
        NEOWPF_ENABLE_CONTROL_STYLES=$<BOOL:${APPLY_CONTROL_STYLES}>
)

neowpf_add_xaml(
    NeoWPF.Foundation
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Controls/Checkbox/Visual/CheckBoxStyles.xaml"
    NAMESPACE NeoWPF::UI::Controls
    COMPONENT Styles
    PUBLIC_HEADER NeoWPF/UI/Controls/Styles.h
)
neowpf_add_xaml(
    NeoWPF.Foundation
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Controls/Button/Visual/ButtonStyles.xaml"
    NAMESPACE NeoWPF::UI::Generated
    COMPONENT ButtonStyles
    PUBLIC_HEADER NeoWPF/UI/Controls/ButtonStyles.h
)
neowpf_add_xaml(
    NeoWPF.Foundation
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Controls/ScrollBar/Visual/ScrollBarStyles.xaml"
    NAMESPACE NeoWPF::UI::Generated
    COMPONENT ScrollBarStyles
    PUBLIC_HEADER NeoWPF/UI/Controls/ScrollBarStyles.h
)
neowpf_add_xaml(
    NeoWPF.Foundation
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Controls/Text/Visual/TextBlockStyles.xaml"
    NAMESPACE NeoWPF::UI::Generated
    COMPONENT TextBlockStyles
    PUBLIC_HEADER NeoWPF/UI/Controls/TextBlockStyles.h
)
neowpf_add_xaml(
    NeoWPF.Foundation
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Controls/TextBox/Visual/TextBoxStyles.xaml"
    NAMESPACE NeoWPF::UI::Generated
    COMPONENT TextBoxStyles
    PUBLIC_HEADER NeoWPF/UI/Controls/TextBoxStyles.h
)
neowpf_add_xaml(
    NeoWPF.Foundation
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Layout/Border/Visual/BorderStyles.xaml"
    NAMESPACE NeoWPF::UI::Generated
    COMPONENT BorderStyles
    PUBLIC_HEADER NeoWPF/UI/Layout/BorderStyles.h
)
neowpf_add_xaml(
    NeoWPF.Foundation
    "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Layout/Panel/Panel.Styles.xaml"
    NAMESPACE NeoWPF::UI::Generated
    COMPONENT PanelStyles
    PUBLIC_HEADER NeoWPF/UI/Layout/PanelStyles.h
)
set(NEOWPF_STYLE_UMBRELLA "${CMAKE_CURRENT_BINARY_DIR}/generated/xaml/NeoWPF/ControlStyles.h")
file(GENERATE OUTPUT "${NEOWPF_STYLE_UMBRELLA}" CONTENT [=[
#pragma once

#if defined(NEOWPF_ENABLE_CONTROL_STYLES) && NEOWPF_ENABLE_CONTROL_STYLES
#if __has_include(<NeoWPF/UI/Controls/Styles.h>)
#include <NeoWPF/UI/Controls/Styles.h>
#endif
#if __has_include(<NeoWPF/UI/Controls/ButtonStyles.h>)
#include <NeoWPF/UI/Controls/ButtonStyles.h>
#endif
#if __has_include(<NeoWPF/UI/Controls/ScrollBarStyles.h>)
#include <NeoWPF/UI/Controls/ScrollBarStyles.h>
#endif
#if __has_include(<NeoWPF/UI/Controls/TextBlockStyles.h>)
#include <NeoWPF/UI/Controls/TextBlockStyles.h>
#endif
#if __has_include(<NeoWPF/UI/Controls/TextBoxStyles.h>)
#include <NeoWPF/UI/Controls/TextBoxStyles.h>
#endif
#if __has_include(<NeoWPF/UI/Layout/BorderStyles.h>)
#include <NeoWPF/UI/Layout/BorderStyles.h>
#endif
#if __has_include(<NeoWPF/UI/Layout/PanelStyles.h>)
#include <NeoWPF/UI/Layout/PanelStyles.h>
#endif
#define APPLY_CONTROL_STYLES(style_type, element) \
do { \
    style_type generated_styles; \
    generated_styles.InitializeComponent(); \
    if (generated_styles.Root) { \
        generated_styles.Root->ApplyTo(element); \
    } \
} while (false)
#else
#define APPLY_CONTROL_STYLES(...) do { } while (false)
#endif
]=])
if(MSVC)
    target_compile_options(NeoWPF.Foundation PRIVATE "/FI${NEOWPF_STYLE_UMBRELLA}")
else()
    target_compile_options(NeoWPF.Foundation PRIVATE "-include${NEOWPF_STYLE_UMBRELLA}")
endif()
if(WIN32)
    target_include_directories(NeoWPF.Foundation PRIVATE "${NEOWPF_VCPKG_TRIPLET}/include")
    target_include_directories(NeoWPF.Foundation PRIVATE "${NEOWPF_VCPKG_TRIPLET}/include/harfbuzz")
endif()
target_include_directories(NeoWPF.Foundation PRIVATE "${Stb_INCLUDE_DIR}")
if(UNIX)
    target_include_directories(NeoWPF.Foundation SYSTEM PRIVATE
        "${NEOWPF_WAYLAND_GENERATED_DIR}"
        "${NEOWPF_VULKAN_GENERATED_DIR}"
    )
endif()

target_compile_definitions(NeoWPF.Foundation
    PUBLIC
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Debug>:NEOWPF_ENABLE_RUNTIME_XAML>
        $<$<CONFIG:Release>:NDEBUG>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    PRIVATE
        NEOWPF_INTERNAL_BUILD
)

if(WIN32)
    target_compile_definitions(NeoWPF.Foundation PRIVATE
        _CONSOLE
        UNICODE
        _UNICODE
        NOMINMAX
    )
endif()

if(UNIX)
    list(APPEND NEOWPF_HEADERS "${NEOWPF_XDG_SHELL_HEADER}")
    list(APPEND NEOWPF_HEADERS "${NEOWPF_XDG_DECORATION_HEADER}")
    list(APPEND NEOWPF_HEADERS "${NEOWPF_SOLID_VERT_HEADER}" "${NEOWPF_SOLID_FRAG_HEADER}")
    list(APPEND NEOWPF_HEADERS "${NEOWPF_TEXTURED_VERT_HEADER}" "${NEOWPF_TEXTURED_FRAG_HEADER}")
    add_custom_target(NeoWPF.GeneratedAssets
        DEPENDS
            "${NEOWPF_XDG_SHELL_HEADER}"
            "${NEOWPF_XDG_SHELL_SOURCE}"
            "${NEOWPF_XDG_DECORATION_HEADER}"
            "${NEOWPF_XDG_DECORATION_SOURCE}"
            "${NEOWPF_SOLID_VERT_HEADER}"
            "${NEOWPF_SOLID_FRAG_HEADER}"
            "${NEOWPF_TEXTURED_VERT_HEADER}"
            "${NEOWPF_TEXTURED_FRAG_HEADER}"
    )
    add_dependencies(NeoWPF.Foundation NeoWPF.GeneratedAssets)
endif()

add_library(NeoWPF STATIC)
add_library(NeoWPF::NeoWPF ALIAS NeoWPF)
target_sources(NeoWPF PRIVATE
    $<TARGET_OBJECTS:NeoWPF.Foundation>
    $<TARGET_OBJECTS:NeoNaml>
)

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
endif()

if(WIN32)
    set(NEOWPF_RELEASE_EXTENSION "lib")
else()
    set(NEOWPF_RELEASE_EXTENSION "a")
endif()

if(UNIX)
    set(_neowpf_windows_repo_root "/mnt/c/Users/Katsu/source/repos/NeoWPF")
    if(EXISTS "${_neowpf_windows_repo_root}")
        set(NEOWPF_RELEASE_DIRECTORY "${_neowpf_windows_repo_root}/NWpfBuild/${NEOWPF_PLATFORM}/$<CONFIG>")
        set(NEOWPF_RELEASE_DIRECTORY_DEBUG "${_neowpf_windows_repo_root}/NWpfBuild/${NEOWPF_PLATFORM}/Debug")
        set(NEOWPF_RELEASE_DIRECTORY_RELEASE "${_neowpf_windows_repo_root}/NWpfBuild/${NEOWPF_PLATFORM}/Release")
        set(NEOWPF_RELEASE_DIRECTORY_RELWITHDEBINFO "${_neowpf_windows_repo_root}/NWpfBuild/${NEOWPF_PLATFORM}/RelWithDebInfo")
        set(NEOWPF_RELEASE_DIRECTORY_MINSIZEREL "${_neowpf_windows_repo_root}/NWpfBuild/${NEOWPF_PLATFORM}/MinSizeRel")
    else()
        set(NEOWPF_RELEASE_DIRECTORY "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/$<CONFIG>")
        set(NEOWPF_RELEASE_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/Debug")
        set(NEOWPF_RELEASE_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/Release")
        set(NEOWPF_RELEASE_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/RelWithDebInfo")
        set(NEOWPF_RELEASE_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/MinSizeRel")
    endif()
else()
    set(NEOWPF_RELEASE_DIRECTORY "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/$<CONFIG>")
    set(NEOWPF_RELEASE_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/Debug")
    set(NEOWPF_RELEASE_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/Release")
    set(NEOWPF_RELEASE_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/RelWithDebInfo")
    set(NEOWPF_RELEASE_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/MinSizeRel")
endif()
set_target_properties(NeoWPF PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${NEOWPF_RELEASE_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${NEOWPF_RELEASE_DIRECTORY_DEBUG}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${NEOWPF_RELEASE_DIRECTORY_RELEASE}"
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${NEOWPF_RELEASE_DIRECTORY_RELWITHDEBINFO}"
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${NEOWPF_RELEASE_DIRECTORY_MINSIZEREL}"
)
set(NEOWPF_RELEASE_SOURCE "$<TARGET_FILE:NeoWPF>")
if(UNIX)
    set_target_properties(NeoWPF PROPERTIES OUTPUT_NAME "NeoWPF")
endif()

target_include_directories(NeoWPF
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
if(WIN32)
    target_include_directories(NeoWPF PRIVATE "${NEOWPF_VCPKG_TRIPLET}/include")
    target_include_directories(NeoWPF PRIVATE "${NEOWPF_VCPKG_TRIPLET}/include/harfbuzz")
endif()
target_include_directories(NeoWPF PRIVATE "${Stb_INCLUDE_DIR}")
if(UNIX)
    target_include_directories(NeoWPF SYSTEM PRIVATE "${NEOWPF_WAYLAND_GENERATED_DIR}" "${NEOWPF_VULKAN_GENERATED_DIR}")
endif()

target_compile_definitions(NeoWPF
    PUBLIC
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Debug>:NEOWPF_ENABLE_RUNTIME_XAML>
        $<$<CONFIG:Release>:NDEBUG>
        $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
    PRIVATE
        NEOWPF_INTERNAL_BUILD
)

if(WIN32)
    target_compile_definitions(NeoWPF PRIVATE
        _CONSOLE
        UNICODE
        _UNICODE
        NOMINMAX
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

    target_compile_options(NeoWPF.Foundation PRIVATE
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
        PUBLIC
            Freetype::Freetype
            harfbuzz::harfbuzz
            CURL::libcurl
            d3d11
            dxgi
            dbghelp
            ole32
            windowscodecs
            bcrypt
            crypt32
            ws2_32
            wldap32
            winmm
            advapi32
            iphlpapi
            normaliz
            secur32
            dwmapi
    )
    target_link_libraries(NeoWPF.Foundation
        PUBLIC
            Freetype::Freetype
            harfbuzz::harfbuzz
            CURL::libcurl
            d3d11
            dxgi
            dbghelp
            ole32
            windowscodecs
            bcrypt
            crypt32
            ws2_32
            wldap32
            winmm
            advapi32
            iphlpapi
            normaliz
            secur32
            dwmapi
    )
else()
    target_link_libraries(NeoWPF
        PUBLIC
            Vulkan::Vulkan
            PkgConfig::WAYLAND
            PkgConfig::XKBCOMMON
            Freetype::Freetype
            PkgConfig::HARFBUZZ
            PkgConfig::FONTCONFIG
            CURL::libcurl
            PNG::PNG
    )
    target_link_libraries(NeoWPF.Foundation
        PUBLIC
            Vulkan::Vulkan
            PkgConfig::WAYLAND
            PkgConfig::XKBCOMMON
            Freetype::Freetype
            PkgConfig::HARFBUZZ
            PkgConfig::FONTCONFIG
            CURL::libcurl
            PNG::PNG
    )
    message(STATUS "NeoWPF Linux configuration selected with Vulkan renderer backend.")
endif()

if(NEOWPF_STATIC_DEPENDENCIES)
    set(NEOWPF_STATIC_DEPENDENCY_TARGETS
        CURL::libcurl
        MbedTLS::mbedcrypto
        MbedTLS::mbedx509
        MbedTLS::mbedtls
        OpenSSL::Crypto
        OpenSSL::SSL
        ZLIB::ZLIB
        PNG::PNG
        Vulkan::Vulkan
        PkgConfig::WAYLAND
        PkgConfig::XKBCOMMON
        Freetype::Freetype
        PkgConfig::HARFBUZZ
        PkgConfig::FONTCONFIG
    )
    set(NEOWPF_STATIC_ARCHIVES_DEBUG)
    set(NEOWPF_STATIC_ARCHIVES_RELEASE)
    foreach(NEOWPF_DEPENDENCY_TARGET IN LISTS NEOWPF_STATIC_DEPENDENCY_TARGETS)
        if(NOT TARGET "${NEOWPF_DEPENDENCY_TARGET}")
            continue()
        endif()

        get_target_property(NEOWPF_DEPENDENCY_DEBUG
            "${NEOWPF_DEPENDENCY_TARGET}" IMPORTED_LOCATION_DEBUG)
        get_target_property(NEOWPF_DEPENDENCY_RELEASE
            "${NEOWPF_DEPENDENCY_TARGET}" IMPORTED_LOCATION_RELEASE)
        get_target_property(NEOWPF_DEPENDENCY_LOCATION
            "${NEOWPF_DEPENDENCY_TARGET}" IMPORTED_LOCATION)
        if(NOT NEOWPF_DEPENDENCY_DEBUG)
            set(NEOWPF_DEPENDENCY_DEBUG "${NEOWPF_DEPENDENCY_LOCATION}")
        endif()
        if(NOT NEOWPF_DEPENDENCY_RELEASE)
            set(NEOWPF_DEPENDENCY_RELEASE "${NEOWPF_DEPENDENCY_LOCATION}")
        endif()

        if(NEOWPF_DEPENDENCY_DEBUG MATCHES "\\.(a|lib)$")
            list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG "${NEOWPF_DEPENDENCY_DEBUG}")
        endif()
        if(NEOWPF_DEPENDENCY_RELEASE MATCHES "\\.(a|lib)$")
            list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE "${NEOWPF_DEPENDENCY_RELEASE}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_DEBUG)
    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_RELEASE)

    foreach(NEOWPF_STATIC_LIBRARY_SPEC IN ITEMS
        "wayland-client;${WAYLAND_LIBRARY_DIRS}"
        "xkbcommon;${XKBCOMMON_LIBRARY_DIRS}"
        "freetype;${FREETYPE_LIBRARY_DIRS}"
        "harfbuzz;${HARFBUZZ_LIBRARY_DIRS}"
        "fontconfig;${FONTCONFIG_LIBRARY_DIRS}")
        list(GET NEOWPF_STATIC_LIBRARY_SPEC 0 NEOWPF_STATIC_LIBRARY_NAME)
        list(REMOVE_AT NEOWPF_STATIC_LIBRARY_SPEC 0)
        find_file(NEOWPF_STATIC_LIBRARY
            "lib${NEOWPF_STATIC_LIBRARY_NAME}.a"
            PATHS ${NEOWPF_STATIC_LIBRARY_SPEC}
            NO_DEFAULT_PATH)
        if(NEOWPF_STATIC_LIBRARY)
            list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG "${NEOWPF_STATIC_LIBRARY}")
            list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE "${NEOWPF_STATIC_LIBRARY}")
        endif()
        unset(NEOWPF_STATIC_LIBRARY CACHE)
    endforeach()

    set(NEOWPF_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    foreach(NEOWPF_TRANSITIVE_LIBRARY_NAME IN ITEMS
        bz2
        z
        png16
        brotlidec
        brotlicommon
        expat)
        find_library(NEOWPF_TRANSITIVE_LIBRARY
            NAMES "${NEOWPF_TRANSITIVE_LIBRARY_NAME}")
        if(NEOWPF_TRANSITIVE_LIBRARY)
            list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG "${NEOWPF_TRANSITIVE_LIBRARY}")
            list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE "${NEOWPF_TRANSITIVE_LIBRARY}")
        endif()
        unset(NEOWPF_TRANSITIVE_LIBRARY CACHE)
    endforeach()
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${NEOWPF_FIND_LIBRARY_SUFFIXES}")
    unset(NEOWPF_FIND_LIBRARY_SUFFIXES)

    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_DEBUG)
    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_RELEASE)

    if(WIN32)
        set(NEOWPF_WINDOWS_SYSTEM_LIBRARIES "")
        foreach(NEOWPF_SYSTEM_LIBRARY_NAME IN ITEMS
            d3d11
            dxgi
            dbghelp
            ole32
            windowscodecs
            bcrypt
            crypt32
            ws2_32
            wldap32
            winmm
            advapi32
            iphlpapi
            normaliz
            secur32
            dwmapi)
            find_library(NEOWPF_SYSTEM_LIBRARY
                NAMES "${NEOWPF_SYSTEM_LIBRARY_NAME}")
            if(NEOWPF_SYSTEM_LIBRARY)
                list(APPEND NEOWPF_WINDOWS_SYSTEM_LIBRARIES "${NEOWPF_SYSTEM_LIBRARY}")
            endif()
            unset(NEOWPF_SYSTEM_LIBRARY CACHE)
        endforeach()

        set(NEOWPF_WINDOWS_MERGE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/merge-neowpf-libraries-$<CONFIG>.cmake")
        string(CONCAT NEOWPF_WINDOWS_MERGE_SCRIPT_CONTENT
            "set(NEOWPF_ARCHIVER [==[${CMAKE_AR}]==])\n"
            "set(NEOWPF_TARGET [==[$<TARGET_FILE:NeoWPF>]==])\n"
            "set(NEOWPF_TRIPLET [==[${NEOWPF_VCPKG_TRIPLET}]==])\n"
            "set(NEOWPF_SYSTEM_LIBRARIES [==[${NEOWPF_WINDOWS_SYSTEM_LIBRARIES}]==])\n"
            "if(\"$<CONFIG>\" STREQUAL \"Debug\")\n"
            "    set(NEOWPF_LIBRARY_DIR \"\${NEOWPF_TRIPLET}/debug/lib\")\n"
            "else()\n"
            "    set(NEOWPF_LIBRARY_DIR \"\${NEOWPF_TRIPLET}/lib\")\n"
            "endif()\n"
            "file(GLOB_RECURSE NEOWPF_DEPENDENCY_LIBRARIES \"\${NEOWPF_LIBRARY_DIR}/*.lib\")\n"
            "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy \"\${NEOWPF_TARGET}\" \"\${NEOWPF_TARGET}.base\" RESULT_VARIABLE NEOWPF_COPY_RESULT)\n"
            "if(NEOWPF_COPY_RESULT)\n"
            "    message(FATAL_ERROR \"NeoWPF archive staging failed with exit code \${NEOWPF_COPY_RESULT}\")\n"
            "endif()\n"
            "execute_process(COMMAND \"\${NEOWPF_ARCHIVER}\" \"/OUT:\${NEOWPF_TARGET}\" \"/IGNORE:4006\" \"\${NEOWPF_TARGET}.base\" \${NEOWPF_DEPENDENCY_LIBRARIES} \${NEOWPF_SYSTEM_LIBRARIES} RESULT_VARIABLE NEOWPF_ARCHIVE_RESULT)\n"
            "file(REMOVE \"\${NEOWPF_TARGET}.base\")\n"
            "if(NEOWPF_ARCHIVE_RESULT)\n"
            "    message(FATAL_ERROR \"NeoWPF dependency merge failed with exit code \${NEOWPF_ARCHIVE_RESULT}\")\n"
            "endif()\n"
        )
        file(GENERATE OUTPUT "${NEOWPF_WINDOWS_MERGE_SCRIPT}" CONTENT
            "${NEOWPF_WINDOWS_MERGE_SCRIPT_CONTENT}"
        )
        add_custom_command(TARGET NeoWPF POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -P "${NEOWPF_WINDOWS_MERGE_SCRIPT}"
            VERBATIM
        )
    elseif(UNIX)
        set(NEOWPF_RELEASE_ARCHIVE
            "${NEOWPF_RELEASE_DIRECTORY}/NeoWPF.${NEOWPF_PLATFORM}.$<CONFIG>.${PROJECT_VERSION}.a")
        set(NEOWPF_ARCHIVE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/merge-neowpf-archives-$<CONFIG>.mri")

        string(CONCAT NEOWPF_ARCHIVE_SCRIPT_CONTENT
            "create ${NEOWPF_RELEASE_ARCHIVE}\n"
            "addlib $<TARGET_FILE:NeoWPF>\n"
        )
        foreach(NEOWPF_STATIC_ARCHIVE IN LISTS NEOWPF_STATIC_ARCHIVES_RELEASE)
            string(APPEND NEOWPF_ARCHIVE_SCRIPT_CONTENT
                "addlib ${NEOWPF_STATIC_ARCHIVE}\n")
        endforeach()
        string(APPEND NEOWPF_ARCHIVE_SCRIPT_CONTENT "save\nend\n")
        file(GENERATE OUTPUT "${NEOWPF_ARCHIVE_SCRIPT}" CONTENT
            "${NEOWPF_ARCHIVE_SCRIPT_CONTENT}"
        )

        add_custom_command(TARGET NeoWPF POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_RELEASE_DIRECTORY}"
            COMMAND sh -c "\"${CMAKE_AR}\" -M < \"${NEOWPF_ARCHIVE_SCRIPT}\""
            VERBATIM
        )
    endif()
endif()

if(WIN32)
    add_custom_command(TARGET NeoWPF POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_RELEASE_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${NEOWPF_RELEASE_SOURCE}"
            "${NEOWPF_RELEASE_DIRECTORY}/NeoWPF.${NEOWPF_PLATFORM}.$<CONFIG>.${PROJECT_VERSION}.${NEOWPF_RELEASE_EXTENSION}"
        VERBATIM
    )
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
        tests/Automated/NamlArtifactTests.cpp
        tests/Automated/EventTests.cpp
        tests/Automated/DependencyPropertyTests.cpp
        tests/Automated/TransformTests.cpp
        tests/Automated/LayoutTests.cpp
        tests/Automated/TransformCompositionTests.cpp
        tests/Automated/RenderCommandTests.cpp
        tests/Automated/GridTests.cpp
        tests/Automated/ItemsControlTests.cpp
        tests/Automated/ListBoxTests.cpp
        tests/Automated/LImageDecodingTests.cpp
        tests/Automated/TriggerTests.cpp
        tests/Automated/XamlParserTests.cpp
    )

    if(UNIX)
        target_include_directories(NeoWPF.AutomatedTests SYSTEM PRIVATE
            "${NEOWPF_WAYLAND_GENERATED_DIR}"
            "${NEOWPF_VULKAN_GENERATED_DIR}"
        )
    endif()

    target_link_libraries(NeoWPF.AutomatedTests
        PRIVATE
            NeoWPF::NeoWPF
            GTest::gtest_main
    )

    target_compile_definitions(NeoWPF.AutomatedTests PRIVATE
        NEOWPF_INTERNAL_BUILD
        NEOWPF_ENABLE_RUNTIME_XAML
    )

    include(GoogleTest)
    gtest_discover_tests(NeoWPF.AutomatedTests)
endif()

install(TARGETS NeoWPF ARCHIVE DESTINATION lib)
install(DIRECTORY include/NeoWPF DESTINATION include)
install(DIRECTORY include/NeoNaml DESTINATION include)
install(FILES LICENSE DESTINATION .)