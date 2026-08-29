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

if(UNIX)
    file(GLOB NEOWPF_VCPKG_PREFIXES LIST_DIRECTORIES true
        "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/*")
    foreach(NEOWPF_VCPKG_PREFIX IN LISTS NEOWPF_VCPKG_PREFIXES)
        if(EXISTS "${NEOWPF_VCPKG_PREFIX}/include/curl/curl.h" AND
           EXISTS "${NEOWPF_VCPKG_PREFIX}/include/stb_image.h" AND
           EXISTS "${NEOWPF_VCPKG_PREFIX}/share/curl/CURLConfig.cmake" AND
           EXISTS "${NEOWPF_VCPKG_PREFIX}/share/stb/FindStb.cmake" AND
           EXISTS "${NEOWPF_VCPKG_PREFIX}/share/freetype/freetype-config.cmake")
            list(PREPEND CMAKE_PREFIX_PATH "${NEOWPF_VCPKG_PREFIX}")
            list(PREPEND CMAKE_MODULE_PATH "${NEOWPF_VCPKG_PREFIX}/share/stb")
            break()
        endif()
    endforeach()
endif()

find_package(CURL CONFIG REQUIRED)
find_package(Stb REQUIRED)
if(UNIX)
    find_package(Freetype CONFIG REQUIRED)
    find_package(PNG REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NEOWPF_STATIC_DEPENDENCIES
    "Merge static NeoWPF dependencies into the NeoWPF archive"
    ON)

include(cmake/NeoWPFSources.cmake)
if(UNIX)
    list(APPEND NEOWPF_HEADERS "${NEOWPF_XDG_SHELL_HEADER}")
    list(APPEND NEOWPF_HEADERS "${NEOWPF_XDG_DECORATION_HEADER}")
    list(APPEND NEOWPF_HEADERS "${NEOWPF_SOLID_VERT_HEADER}" "${NEOWPF_SOLID_FRAG_HEADER}")
    list(APPEND NEOWPF_HEADERS "${NEOWPF_TEXTURED_VERT_HEADER}" "${NEOWPF_TEXTURED_FRAG_HEADER}")
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
if(UNIX)
    set_target_properties(NeoWPF PROPERTIES OUTPUT_NAME "NeoWPF.objects")
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
    target_include_directories(NeoWPF PRIVATE "${NEOWPF_WAYLAND_GENERATED_DIR}" "${NEOWPF_VULKAN_GENERATED_DIR}")
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
            ole32
            windowscodecs
            CURL::libcurl
    )
else()
    target_link_libraries(NeoWPF
        PUBLIC
            Vulkan::Vulkan
            PkgConfig::WAYLAND
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
        ZLIB::ZLIB
        PNG::PNG
        Vulkan::Vulkan
        PkgConfig::WAYLAND
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

    foreach(NEOWPF_STATIC_LIBRARY_SPEC IN ITEMS
        "wayland-client;${WAYLAND_LIBRARY_DIRS}"
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
    if(NOT NEOWPF_STATIC_ARCHIVES_RELEASE)
        message(FATAL_ERROR "No static Linux dependency archives were found for NeoWPF.")
    endif()

    if(WIN32)
        add_custom_command(TARGET NeoWPF POST_BUILD
            COMMAND "${CMAKE_AR}"
                "/OUT:$<TARGET_FILE:NeoWPF>.merged"
                "$<TARGET_FILE:NeoWPF>"
                "$<IF:$<CONFIG:Debug>,${NEOWPF_STATIC_ARCHIVES_DEBUG},${NEOWPF_STATIC_ARCHIVES_RELEASE}>"
            COMMAND "${CMAKE_COMMAND}" -E copy
                "$<TARGET_FILE:NeoWPF>.merged"
                "$<TARGET_FILE:NeoWPF>"
            COMMAND "${CMAKE_COMMAND}" -E rm -f "$<TARGET_FILE:NeoWPF>.merged"
            VERBATIM
        )
    elseif(UNIX)
        set(NEOWPF_ARCHIVE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/merge-neowpf-archives-$<CONFIG>.mri")
        set(NEOWPF_LINKER_SCRIPT "$<TARGET_FILE_DIR:NeoWPF>/libNeoWPF.a")
        get_target_property(NEOWPF_CURL_DEBUG CURL::libcurl IMPORTED_LOCATION_DEBUG)
        get_target_property(NEOWPF_CURL_RELEASE CURL::libcurl IMPORTED_LOCATION_RELEASE)
        if(NOT NEOWPF_CURL_DEBUG)
            set(NEOWPF_CURL_DEBUG "${NEOWPF_CURL_RELEASE}")
        endif()
        if(NOT NEOWPF_CURL_RELEASE)
            set(NEOWPF_CURL_RELEASE "${NEOWPF_CURL_DEBUG}")
        endif()
        if(NOT NEOWPF_CURL_DEBUG OR NOT NEOWPF_CURL_RELEASE)
            message(FATAL_ERROR "Could not determine the vcpkg curl archive for the Linux linker wrapper.")
        endif()
        string(CONCAT NEOWPF_ARCHIVE_SCRIPT_CONTENT
            "create $<TARGET_FILE:NeoWPF>.merged\n"
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
        file(GENERATE OUTPUT "${NEOWPF_LINKER_SCRIPT}" CONTENT
            "GROUP ( $<TARGET_FILE:NeoWPF> -lvulkan -lwayland-client -lharfbuzz -lfontconfig -lfreetype -lpng16 -lz -lbrotlidec -lbrotlicommon -lbz2 -lexpat $<IF:$<CONFIG:Debug>,${NEOWPF_CURL_DEBUG},${NEOWPF_CURL_RELEASE}> -ldl -lpthread )\n"
        )
        add_custom_command(TARGET NeoWPF POST_BUILD
            COMMAND sh -c "\"${CMAKE_AR}\" -M < \"${NEOWPF_ARCHIVE_SCRIPT}\""
            COMMAND "${CMAKE_COMMAND}" -E copy
                "$<TARGET_FILE:NeoWPF>.merged"
                "$<TARGET_FILE:NeoWPF>"
            COMMAND "${CMAKE_COMMAND}" -E rm -f "$<TARGET_FILE:NeoWPF>.merged"
            VERBATIM
        )
    endif()
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
        tests/Automated/TransformTests.cpp
        tests/Automated/LayoutTests.cpp
        tests/Automated/TransformCompositionTests.cpp
        tests/Automated/RenderCommandTests.cpp
        tests/Automated/GridTests.cpp
        tests/Automated/LImageDecodingTests.cpp
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
