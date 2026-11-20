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
        elseif(ANDROID)
            set(NEOWPF_PLATFORM_NAME "Android")
            set(NEOWPF_LIBRARY_EXTENSION "a")
        elseif(UNIX)
            set(NEOWPF_PLATFORM_NAME "Linux")
            set(NEOWPF_LIBRARY_EXTENSION "a")
        else()
            message(FATAL_ERROR "Unsupported NeoWPF platform: ${CMAKE_SYSTEM_NAME}")
        endif()

        if(ANDROID)
            set(NEOWPF_LIBRARY
                "${NEOWPF_ROOT}/NWpfBuild/Android/${CMAKE_BUILD_TYPE}/libNeoWPF.a")
        elseif(CMAKE_CONFIGURATION_TYPES)
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

    if(ANDROID)
        set(NEOWPF_CONTROL_GALLERY_TARGET neowpf_android)
        add_library(${NEOWPF_CONTROL_GALLERY_TARGET} SHARED
            ${CONTROL_GALLERY_SOURCES}
            ${CONTROL_GALLERY_HEADERS}
        )
    else()
        set(NEOWPF_CONTROL_GALLERY_TARGET NeoWPF.ControlGallerySample)
        add_executable(${NEOWPF_CONTROL_GALLERY_TARGET}
            ${CONTROL_GALLERY_SOURCES}
            ${CONTROL_GALLERY_HEADERS}
        )
    endif()

    target_include_directories(${NEOWPF_CONTROL_GALLERY_TARGET} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Gallery"
        "${NEOWPF_ROOT}/include"
    )

    if(ANDROID)
        set_source_files_properties(
            "${CMAKE_CURRENT_SOURCE_DIR}/src/App.cpp"
            PROPERTIES COMPILE_DEFINITIONS "main=NeoWPFAndroidApplicationMain"
        )
        target_link_options(${NEOWPF_CONTROL_GALLERY_TARGET} PRIVATE
            "-Wl,-u,ANativeActivity_onCreate"
        )
        target_link_libraries(${NEOWPF_CONTROL_GALLERY_TARGET} PRIVATE
            "${NEOWPF_LIBRARY}"
            vulkan
            android
            log
            "${CMAKE_DL_LIBS}"
        )
    elseif(UNIX)
        find_package(PkgConfig REQUIRED)
        find_package(Vulkan REQUIRED)
        pkg_check_modules(WAYLAND REQUIRED IMPORTED_TARGET wayland-client)
        pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)

        target_link_libraries(${NEOWPF_CONTROL_GALLERY_TARGET} PRIVATE
            "${NEOWPF_LIBRARY}"
            Vulkan::Vulkan
            PkgConfig::WAYLAND
            PkgConfig::XKBCOMMON
        )
    else()
        target_link_libraries(${NEOWPF_CONTROL_GALLERY_TARGET} PRIVATE "${NEOWPF_LIBRARY}")
    endif()
else()
    add_custom_target(NeoWPF.ControlGallerySample)
    message(STATUS "No ControlGallery sources found yet; executable target will be added when src/*.cpp files exist.")
endif()
