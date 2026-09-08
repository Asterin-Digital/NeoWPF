function(neowpf_add_xaml TARGET_NAME INPUT_XAML)
    set(options)
    set(oneValueArgs NAMESPACE COMPONENT PUBLIC_HEADER)
    cmake_parse_arguments(NEOWPF_XAML "${options}" "${oneValueArgs}" "" ${ARGN})

    get_filename_component(_xaml_name "${INPUT_XAML}" NAME_WE)
    set(_output_directory "${CMAKE_CURRENT_BINARY_DIR}/generated/xaml/${TARGET_NAME}")
    set(_generated_header "${_output_directory}/${_xaml_name}.xaml.g.h")
    set(_generated_source "${_output_directory}/${_xaml_name}.xaml.g.cpp")
    set(_generated_artifact "${_output_directory}/${_xaml_name}.xaml.g.naml")
    set(_public_header)
    if(NEOWPF_XAML_PUBLIC_HEADER)
        set(_public_header "${CMAKE_CURRENT_BINARY_DIR}/generated/xaml/${NEOWPF_XAML_PUBLIC_HEADER}")
        get_filename_component(_public_header_directory "${_public_header}" DIRECTORY)
        file(MAKE_DIRECTORY "${_public_header_directory}")
        file(WRITE "${_public_header}"
            "#pragma once\n"
            "#include <${_xaml_name}.xaml.g.h>\n"
        )
    endif()

    add_custom_command(
        OUTPUT "${_generated_header}" "${_generated_source}" "${_generated_artifact}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_output_directory}"
        COMMAND ${CMAKE_COMMAND}
            -DCOMPILER=${NEOWPF_XAML_COMPILER}
            -DINPUT=${INPUT_XAML}
            -DOUTPUT=${_output_directory}
            -DNAMESPACE=${NEOWPF_XAML_NAMESPACE}
            -DCOMPONENT=${NEOWPF_XAML_COMPONENT}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunXamlCompiler.cmake"
        DEPENDS
            "${INPUT_XAML}"
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunXamlCompiler.cmake"
        COMMENT "Generating NeoWPF XAML: ${INPUT_XAML}"
        VERBATIM
    )
    set_source_files_properties(
        "${_generated_header}" "${_generated_source}" "${_generated_artifact}" ${_public_header}
        PROPERTIES GENERATED TRUE
    )
    target_include_directories("${TARGET_NAME}" PRIVATE "${_output_directory}")
    target_sources("${TARGET_NAME}" PRIVATE "${_generated_source}")
    if(_public_header)
        target_sources("${TARGET_NAME}" PRIVATE "${_public_header}")
    endif()
endfunction()
