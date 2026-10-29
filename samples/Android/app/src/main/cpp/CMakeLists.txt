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
)

add_library(neowpf_android SHARED
    "${NEOWPF_ROOT}/src/Platform/Android/Jni/AndroidJni.cpp"
)
target_include_directories(neowpf_android PRIVATE
    "${NEOWPF_ROOT}/include"
)
target_link_libraries(neowpf_android PRIVATE
    NeoWPF::NeoWPF
    android
    log
    vulkan
    ${CMAKE_DL_LIBS}
)