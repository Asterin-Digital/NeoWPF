cmake_minimum_required(VERSION 3.21)

project("NeoWPF.ControlGallery" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NEOWPF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "Path to the NeoWPF source tree")
set(NEOWPF_LIBRARY "" CACHE FILEPATH "Path to the NeoWPF library")
if(NOT EXISTS "${NEOWPF_ROOT}/include/NeoWPF")
    message(FATAL_ERROR "NEOWPF_ROOT does not contain include/NeoWPF: ${NEOWPF_ROOT}")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ControlGallerySources.cmake")

if(CONTROL_GALLERY_SOURCES)
    set(NEOWPF_LIBRARY_CONFIG_DEPENDENT OFF)
    if(NOT NEOWPF_LIBRARY)
        include("${NEOWPF_ROOT}/cmake/NeoWPFVersion.cmake")
        neowpf_resolve_version_file("${NEOWPF_ROOT}" NEOWPF_VERSION_FILE)
        neowpf_read_version_file("${NEOWPF_VERSION_FILE}" NEOWPF_ARTIFACT_VERSION)

        if(WIN32)
            set(NEOWPF_PLATFORM_NAME "Windows")
            set(NEOWPF_LIBRARY_EXTENSION "lib")
        elseif(UNIX)
            set(NEOWPF_PLATFORM_NAME "Linux")
            set(NEOWPF_LIBRARY_EXTENSION "a")
        else()
            message(FATAL_ERROR "Unsupported NeoWPF platform: ${CMAKE_SYSTEM_NAME}")
        endif()

        if(CMAKE_CONFIGURATION_TYPES)
            set(NEOWPF_LIBRARY
                "${NEOWPF_ROOT}/NWpfBuild/${NEOWPF_PLATFORM_NAME}/$<CONFIG>/NeoWPF.${NEOWPF_PLATFORM_NAME}.$<CONFIG>.${NEOWPF_ARTIFACT_VERSION}.${NEOWPF_LIBRARY_EXTENSION}")
            set(NEOWPF_LIBRARY_CONFIG_DEPENDENT ON)
        else()
            set(NEOWPF_LIBRARY
                "${NEOWPF_ROOT}/NWpfBuild/${NEOWPF_PLATFORM_NAME}/${CMAKE_BUILD_TYPE}/NeoWPF.${NEOWPF_PLATFORM_NAME}.${CMAKE_BUILD_TYPE}.${NEOWPF_ARTIFACT_VERSION}.${NEOWPF_LIBRARY_EXTENSION}")
        endif()
    endif()

    if(NOT NEOWPF_LIBRARY_CONFIG_DEPENDENT AND NOT EXISTS "${NEOWPF_LIBRARY}")
        message(FATAL_ERROR "NeoWPF library not found: ${NEOWPF_LIBRARY}")
    endif()

    add_executable(NeoWPF.ControlGallerySample
        ${CONTROL_GALLERY_SOURCES}
        ${CONTROL_GALLERY_HEADERS}
    )

    target_include_directories(NeoWPF.ControlGallerySample PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Gallery"
        "${NEOWPF_ROOT}/include"
    )

    if(UNIX)
        find_package(PkgConfig REQUIRED)
        find_package(Vulkan REQUIRED)
        pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
        pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)

        target_link_libraries(NeoWPF.ControlGallerySample PRIVATE
            "${NEOWPF_LIBRARY}"
            Vulkan::Vulkan
            PkgConfig::WAYLAND
            PkgConfig::XKBCOMMON
        )
    else()
        target_link_libraries(NeoWPF.ControlGallerySample PRIVATE "${NEOWPF_LIBRARY}")
    endif()
else()
    add_custom_target(NeoWPF.ControlGallerySample)
    message(STATUS "No ControlGallery sources found yet; executable target will be added when src/*.cpp files exist.")
endif()
