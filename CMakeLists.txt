cmake_minimum_required(VERSION 3.20)

project(NeoWPF LANGUAGES CXX)

if(WIN32)
    set(NEOWPF_PLATFORM "Windows")
elseif(UNIX)
    set(NEOWPF_PLATFORM "Linux")
else()
    message(FATAL_ERROR "Unsupported NeoWPF platform: ${CMAKE_SYSTEM_NAME}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/NeoWPFSources.cmake)

add_library(NeoWPF STATIC ${NEOWPF_SOURCES} ${NEOWPF_HEADERS})
add_library(NeoWPF::NeoWPF ALIAS NeoWPF)

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
        PUBLIC
            d3d11
            dxgi
            dwrite
            dbghelp
            winhttp
            ole32
            windowscodecs
    )
else()
    message(STATUS "NeoWPF Linux configuration selected; Linux platform backends are not implemented yet.")
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${NEOWPF_SOURCES} ${NEOWPF_HEADERS})

install(TARGETS NeoWPF ARCHIVE DESTINATION lib)
install(DIRECTORY include/NeoWPF DESTINATION include)
install(FILES LICENSE DESTINATION .)
