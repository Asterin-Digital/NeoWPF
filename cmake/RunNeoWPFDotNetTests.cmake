if(NOT DEFINED DOTNET_EXECUTABLE OR NOT DEFINED TEST_PROJECT)
    message(FATAL_ERROR "DOTNET_EXECUTABLE and TEST_PROJECT are required")
endif()

foreach(TEST_CONFIGURATION IN ITEMS Debug Release)
    execute_process(
        COMMAND "${DOTNET_EXECUTABLE}" test "${TEST_PROJECT}" --configuration "${TEST_CONFIGURATION}"
        RESULT_VARIABLE TEST_RESULT
    )
    if(NOT TEST_RESULT EQUAL 0)
        message(FATAL_ERROR "NeoWPF.DotNet generator tests failed for ${TEST_CONFIGURATION}")
    endif()
endforeach()
