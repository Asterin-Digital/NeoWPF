include_guard(GLOBAL)

option(NEOWPF_USE_NEOLINT
    "Run the versioned NeoLint Release executable before compiling NeoWPF"
    ON)

if(NOT NEOWPF_USE_NEOLINT)
    message(STATUS "NeoLint checks disabled (set NEOWPF_USE_NEOLINT=ON to enable)")
    return()
endif()

if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL
        "Export compile_commands.json for NeoLint" FORCE)
endif()

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/NeoLint/NeoLintVersion.txt" NEOWPF_NEOLINT_VERSION)
string(STRIP "${NEOWPF_NEOLINT_VERSION}" NEOWPF_NEOLINT_VERSION)
if(NOT NEOWPF_NEOLINT_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message(FATAL_ERROR "NeoLintVersion.txt must contain a semantic version")
endif()

if(NOT DEFINED NEOWPF_NEOLINT_EXECUTABLE OR
   NEOWPF_NEOLINT_EXECUTABLE STREQUAL "")
    if(WIN32)
        set(NEOWPF_NEOLINT_EXECUTABLE
            "${CMAKE_CURRENT_SOURCE_DIR}/NeoLint/NLintBuild/Windows/Release/NeoLint.Windows.Release.${NEOWPF_NEOLINT_VERSION}.exe"
            CACHE FILEPATH
            "Versioned NeoLint Release executable")
    else()
        set(NEOWPF_NEOLINT_EXECUTABLE
            "${CMAKE_CURRENT_SOURCE_DIR}/NeoLint/NLintBuild/Linux/Release/NeoLint.Linux.Release.${NEOWPF_NEOLINT_VERSION}"
            CACHE FILEPATH
            "Versioned NeoLint Release executable")
    endif()
endif()

if(NOT EXISTS "${NEOWPF_NEOLINT_EXECUTABLE}")
    message(FATAL_ERROR
        "NEOWPF_USE_NEOLINT is ON, but the versioned NeoLint Release executable was not found: "
        "${NEOWPF_NEOLINT_EXECUTABLE}. Build NeoLint in Release or set "
        "NEOWPF_NEOLINT_EXECUTABLE to its path.")
endif()

set(_NEOWPF_NEOLINT_DEFAULT_OUTPUT_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/$<CONFIG>")
if(NOT DEFINED NEOWPF_NEOLINT_OUTPUT_DIR OR
   NEOWPF_NEOLINT_OUTPUT_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/build")
    set(NEOWPF_NEOLINT_OUTPUT_DIR "${_NEOWPF_NEOLINT_DEFAULT_OUTPUT_DIR}" CACHE PATH
        "Directory for NeoLint reports and incremental cache" FORCE)
endif()
unset(_NEOWPF_NEOLINT_DEFAULT_OUTPUT_DIR)
set(NEOWPF_NEOLINT_REPORT "${NEOWPF_NEOLINT_OUTPUT_DIR}/neolint-report.json")
set(NEOWPF_NEOLINT_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/.neolint.json")
set(NEOWPF_NEOLINT_COMPILE_COMMANDS
    "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
set(NEOWPF_NEOLINT_CACHE "${NEOWPF_NEOLINT_OUTPUT_DIR}/neolint-cache.json")

if(NOT EXISTS "${NEOWPF_NEOLINT_CONFIG}")
    message(FATAL_ERROR "NeoLint configuration was not found: ${NEOWPF_NEOLINT_CONFIG}")
endif()

if(NOT EXISTS "${NEOWPF_NEOLINT_COMPILE_COMMANDS}")
    message(WARNING
        "NeoLint is enabled, but compile_commands.json does not exist. "
        "Reconfigure NeoWPF before building.")
endif()

add_custom_target(NeoWPF.NeoLintCheck
    COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_NEOLINT_OUTPUT_DIR}"
    COMMAND "${NEOWPF_NEOLINT_EXECUTABLE}"
        --compile-commands "${NEOWPF_NEOLINT_COMPILE_COMMANDS}"
        --config "${NEOWPF_NEOLINT_CONFIG}"
        --report "${NEOWPF_NEOLINT_REPORT}"
        --cache "${NEOWPF_NEOLINT_CACHE}"
        --source-root "${CMAKE_CURRENT_SOURCE_DIR}/src"
        --source-root "${CMAKE_CURRENT_SOURCE_DIR}/include"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Running NeoLint checks before compiling NeoWPF"
    VERBATIM
)

add_dependencies(NeoWPF.Foundation NeoWPF.NeoLintCheck)

message(STATUS "NeoLint checks enabled: ${NEOWPF_NEOLINT_EXECUTABLE}")
