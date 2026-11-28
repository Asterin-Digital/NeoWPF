set(build_command "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --target "${TARGET}")
if(NOT "${CONFIG}" STREQUAL "")
    list(APPEND build_command --config "${CONFIG}")
endif()

execute_process(
    COMMAND ${build_command}
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_output
    ERROR_VARIABLE build_error
)

if("${build_result}" STREQUAL "0")
    message(FATAL_ERROR "The entry-link fixture unexpectedly linked.")
endif()

string(CONCAT combined_output "${build_output}" "${build_error}")
if(NOT combined_output MATCHES "${EXPECTED_PATTERN}")
    message(FATAL_ERROR
        "The entry-link fixture failed for an unexpected reason:\n${combined_output}")
endif()
