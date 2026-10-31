cmake_minimum_required(VERSION 3.22.1)

project(NeoWPFAndroid LANGUAGES CXX)

get_filename_component(NEOWPF_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../.."
    ABSOLUTE
)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
set(NEOWPF_ANDROID_ARCHIVE
    "${NEOWPF_ROOT}/NWpfBuild/Android/${CMAKE_BUILD_TYPE}/libNeoWPF.a")
if(NOT EXISTS "${NEOWPF_ANDROID_ARCHIVE}")
    message(FATAL_ERROR
        "NeoWPF Android archive not found: ${NEOWPF_ANDROID_ARCHIVE}. "
        "Build it with private/scripts/Build.py first.")
endif()

add_library(NeoWPF::NeoWPF STATIC IMPORTED GLOBAL)
set_target_properties(NeoWPF::NeoWPF PROPERTIES
    IMPORTED_LOCATION "${NEOWPF_ANDROID_ARCHIVE}"
    INTERFACE_INCLUDE_DIRECTORIES "${NEOWPF_ROOT}/include"
    INTERFACE_COMPILE_DEFINITIONS "main=NeoWPFAndroidApplicationMain"
    INTERFACE_LINK_OPTIONS "-Wl,-u,ANativeActivity_onCreate"
    INTERFACE_LINK_LIBRARIES "vulkan;android;log;${CMAKE_DL_LIBS}"
)

add_library(neowpf_android SHARED
    "${CMAKE_CURRENT_LIST_DIR}/Main.cpp"
)
target_link_libraries(neowpf_android PRIVATE
    NeoWPF::NeoWPF
)
