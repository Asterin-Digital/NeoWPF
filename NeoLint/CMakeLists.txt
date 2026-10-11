cmake_minimum_required(VERSION 3.20)

project(NeoLint LANGUAGES CXX)

include(CTest)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/NeoLintVersion.txt" NEOLINT_VERSION)
string(STRIP "${NEOLINT_VERSION}" NEOLINT_VERSION)
if(NOT NEOLINT_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
  message(FATAL_ERROR "NeoLintVersion.txt must contain a semantic version")
endif()

if(WIN32)
  set(NEOLINT_PLATFORM "Windows")
  set(NEOLINT_DEFAULT_VCPKG_TRIPLET "x64-windows-static-md")
elseif(UNIX)
  set(NEOLINT_PLATFORM "Linux")
  set(NEOLINT_DEFAULT_VCPKG_TRIPLET "x64-linux")
else()
  message(FATAL_ERROR "Unsupported NeoLint platform: ${CMAKE_SYSTEM_NAME}")
endif()

set(VCPKG_TARGET_TRIPLET "${NEOLINT_DEFAULT_VCPKG_TRIPLET}" CACHE STRING
  "vcpkg triplet used by NeoLint")
set(NEOLINT_VCPKG_PREFIX
  "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
if(NOT EXISTS "${NEOLINT_VCPKG_PREFIX}/share/nlohmann_json/nlohmann_jsonConfig.cmake")
  message(FATAL_ERROR
    "NeoLint requires nlohmann-json for triplet '${VCPKG_TARGET_TRIPLET}'. "
    "Install it with: vcpkg install --triplet ${VCPKG_TARGET_TRIPLET}")
endif()
list(PREPEND CMAKE_PREFIX_PATH "${NEOLINT_VCPKG_PREFIX}")

if(WIN32)
  set(NEOLINT_CLANG_ROOT_ENV_NAME "WINDOWS_CLANG_ROOT")
  set(NEOLINT_CLANG_SDK_PATH "$ENV{WINDOWS_CLANG_ROOT}" CACHE PATH
    "Windows LLVM/Clang SDK prefix")
else()
  set(NEOLINT_CLANG_ROOT_ENV_NAME "LINUX_CLANG_ROOT")
  set(NEOLINT_CLANG_SDK_PATH "$ENV{LINUX_CLANG_ROOT}" CACHE PATH
    "Linux LLVM/Clang SDK prefix")
endif()

if(NOT NEOLINT_CLANG_SDK_PATH)
  message(FATAL_ERROR
    "${NEOLINT_CLANG_ROOT_ENV_NAME} must point to the external LLVM/Clang SDK")
endif()

if(NOT EXISTS "${NEOLINT_CLANG_SDK_PATH}/include/llvm/ADT/StringRef.h" OR
   NOT EXISTS "${NEOLINT_CLANG_SDK_PATH}/include/clang/AST/ASTContext.h")
  message(FATAL_ERROR
    "${NEOLINT_CLANG_ROOT_ENV_NAME} does not contain the LLVM/Clang headers: ${NEOLINT_CLANG_SDK_PATH}")
endif()

set(LLVM_DIR "${NEOLINT_CLANG_SDK_PATH}/share/llvm" CACHE PATH
  "LLVM CMake package directory" FORCE)
set(Clang_DIR "${NEOLINT_CLANG_SDK_PATH}/share/clang" CACHE PATH
  "Clang CMake package directory" FORCE)
find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)
message(STATUS "LLVM/Clang SDK root: ${NEOLINT_CLANG_SDK_PATH}")
find_package(nlohmann_json CONFIG REQUIRED)
if(BUILD_TESTING)
  find_package(GTest CONFIG REQUIRED)
endif()

set(NEOLINT_TEST_CLANG_TOOL clang-tool)
if(UNIX)
  set(NEOLINT_TEST_CLANG_TOOL "${LLVM_TOOLS_BINARY_DIR}/clang++")
endif()

set(NEOLINT_RTTI_COMPILE_OPTION)
if(NOT LLVM_ENABLE_RTTI)
  if(MSVC)
    set(NEOLINT_RTTI_COMPILE_OPTION /GR-)
  else()
    set(NEOLINT_RTTI_COMPILE_OPTION -fno-rtti)
  endif()
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/NeoLintSources.cmake")

add_executable(NeoLint ${NEOLINT_SOURCES})

set(NEOLINT_OUTPUT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/NLintBuild/${NEOLINT_PLATFORM}")
set_target_properties(NeoLint PROPERTIES
  OUTPUT_NAME "NeoLint.${NEOLINT_PLATFORM}.$<CONFIG>.${NEOLINT_VERSION}"
  RUNTIME_OUTPUT_DIRECTORY "${NEOLINT_OUTPUT_ROOT}/$<CONFIG>"
  LIBRARY_OUTPUT_DIRECTORY "${NEOLINT_OUTPUT_ROOT}/$<CONFIG>"
  ARCHIVE_OUTPUT_DIRECTORY "${NEOLINT_OUTPUT_ROOT}/$<CONFIG>"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${NEOLINT_OUTPUT_ROOT}/Debug"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${NEOLINT_OUTPUT_ROOT}/Release"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${NEOLINT_OUTPUT_ROOT}/RelWithDebInfo"
  RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${NEOLINT_OUTPUT_ROOT}/MinSizeRel"
  LIBRARY_OUTPUT_DIRECTORY_DEBUG "${NEOLINT_OUTPUT_ROOT}/Debug"
  LIBRARY_OUTPUT_DIRECTORY_RELEASE "${NEOLINT_OUTPUT_ROOT}/Release"
  LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${NEOLINT_OUTPUT_ROOT}/RelWithDebInfo"
  LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${NEOLINT_OUTPUT_ROOT}/MinSizeRel"
  ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${NEOLINT_OUTPUT_ROOT}/Debug"
  ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${NEOLINT_OUTPUT_ROOT}/Release"
  ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${NEOLINT_OUTPUT_ROOT}/RelWithDebInfo"
  ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${NEOLINT_OUTPUT_ROOT}/MinSizeRel"
)

target_include_directories(NeoLint PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_include_directories(NeoLint SYSTEM PRIVATE
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)
target_compile_definitions(NeoLint PRIVATE ${LLVM_DEFINITIONS})
target_compile_features(NeoLint PRIVATE cxx_std_20)
if(NEOLINT_RTTI_COMPILE_OPTION)
  target_compile_options(NeoLint PRIVATE ${NEOLINT_RTTI_COMPILE_OPTION})
endif()
target_link_libraries(NeoLint PRIVATE
  clangTooling
  clangFrontend
  clangAST
  clangBasic
  LLVMSupport
  nlohmann_json::nlohmann_json
)

if(MSVC)
  target_compile_options(NeoLint PRIVATE /W4 /permissive- /external:anglebrackets /external:W0 /wd4267)
endif()

if(BUILD_TESTING)
  add_executable(NeoLint.Tests
    ${NEOLINT_TEST_SOURCES}
  )
  target_include_directories(NeoLint.Tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  )
  target_compile_definitions(NeoLint.Tests PRIVATE
    NEOLINT_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    NEOLINT_TEST_CLANG_TOOL="${NEOLINT_TEST_CLANG_TOOL}"
  )
  target_compile_features(NeoLint.Tests PRIVATE cxx_std_20)
  if(NEOLINT_RTTI_COMPILE_OPTION)
    target_compile_options(NeoLint.Tests PRIVATE ${NEOLINT_RTTI_COMPILE_OPTION})
  endif()
  target_include_directories(NeoLint.Tests SYSTEM PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
  )
  target_link_libraries(NeoLint.Tests PRIVATE
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    clangTooling
    clangFrontend
    clangASTMatchers
    clangAST
    clangBasic
    LLVMSupport
  )
  if(MSVC)
    target_compile_options(NeoLint.Tests PRIVATE /W4 /permissive- /external:anglebrackets /external:W0 /wd4267)
  endif()
  include(GoogleTest)
  gtest_discover_tests(NeoLint.Tests)
endif()

