foreach(_required_variable IN ITEMS
    NEOWPF_VERSION_FILE
    NEOWPF_BUILT_VERSION_FILE
    NEOWPF_EXPECTED_VERSION
)
    if(NOT DEFINED ${_required_variable})
        message(FATAL_ERROR "${_required_variable} must be provided")
    endif()
endforeach()

get_filename_component(_built_version_directory
    "${NEOWPF_BUILT_VERSION_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${_built_version_directory}")
file(LOCK "${NEOWPF_BUILT_VERSION_FILE}.lock"
    GUARD PROCESS TIMEOUT 30 RESULT_VARIABLE _lock_result)
if(NOT _lock_result STREQUAL "0")
    message(FATAL_ERROR "Could not lock the NeoWPF build version: ${_lock_result}")
endif()

file(READ "${NEOWPF_VERSION_FILE}" _current_version)
string(STRIP "${_current_version}" _current_version)
if(NOT _current_version STREQUAL NEOWPF_EXPECTED_VERSION)
    message(STATUS
        "NeoWPF version changed during this build; leaving the build counter unchanged")
    return()
endif()

if(NOT _current_version MATCHES "^([0-9]+\\.[0-9]+\\.[0-9]+\\.)([0-9]+)$")
    message(FATAL_ERROR "Invalid NeoWPF build version: ${_current_version}")
endif()
if(EXISTS "${NEOWPF_BUILT_VERSION_FILE}")
    file(READ "${NEOWPF_BUILT_VERSION_FILE}" _built_version)
    string(STRIP "${_built_version}" _built_version)
    if(_built_version STREQUAL NEOWPF_EXPECTED_VERSION)
        return()
    endif()
endif()

file(WRITE "${NEOWPF_BUILT_VERSION_FILE}" "${NEOWPF_EXPECTED_VERSION}\n")
message(STATUS "Recorded NeoWPF built version: ${NEOWPF_EXPECTED_VERSION}")