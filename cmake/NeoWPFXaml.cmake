function(neowpf_compile_xaml TARGET_NAME INPUT_XAML)
    set(options)
    set(oneValueArgs NAMESPACE COMPONENT PUBLIC_HEADER)
    set(multiValueArgs)
    cmake_parse_arguments(
        NEOWPF_XAML
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if(NOT TARGET "${TARGET_NAME}")
        message(FATAL_ERROR
            "neowpf_compile_xaml: target does not exist: ${TARGET_NAME}")
    endif()
    if(NOT EXISTS "${INPUT_XAML}")
        message(FATAL_ERROR
            "neowpf_compile_xaml: XAML input does not exist: ${INPUT_XAML}")
    endif()
    if(NOT NEOWPF_XAML_COMPILER)
        message(FATAL_ERROR
            "neowpf_compile_xaml: NEOWPF_XAML_COMPILER must point to XamlCompiler")
    endif()

    get_filename_component(_xaml_filename "${INPUT_XAML}" NAME)
    string(REGEX REPLACE "\\.xaml$" "" _xaml_name "${_xaml_filename}")
    if(NOT NEOWPF_XAML_NAMESPACE)
        set(NEOWPF_XAML_NAMESPACE "NeoWPF::Generated")
    endif()
    if(NOT NEOWPF_XAML_COMPONENT)
        set(NEOWPF_XAML_COMPONENT "${_xaml_name}")
    endif()

    set(_output_directory
        "${CMAKE_CURRENT_BINARY_DIR}/generated/xaml/${TARGET_NAME}")
    set(_generated_name "${_xaml_name}.xaml.g")
    set(_generated_header "${_output_directory}/${_generated_name}.h")
    set(_generated_source "${_output_directory}/${_generated_name}.cpp")
    set(_generated_artifact "${_output_directory}/${_generated_name}.naml")
    if(NEOWPF_XAML_PUBLIC_HEADER)
        set(_public_header "${CMAKE_CURRENT_BINARY_DIR}/generated/xaml/${NEOWPF_XAML_PUBLIC_HEADER}")
        get_filename_component(_public_header_directory "${_public_header}" DIRECTORY)
    endif()

    add_custom_command(
        OUTPUT
            "${_generated_header}"
            "${_generated_source}"
            "${_generated_artifact}"
            ${_public_header}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_output_directory}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_public_header_directory}"
        COMMAND "${NEOWPF_XAML_COMPILER}"
            "${INPUT_XAML}"
            "${_output_directory}"
            --namespace "${NEOWPF_XAML_NAMESPACE}"
            --component "${NEOWPF_XAML_COMPONENT}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_generated_header}"
            "${_public_header}"
        DEPENDS
            "${INPUT_XAML}"
        COMMENT "Compiling NeoWPF XAML: ${INPUT_XAML}"
        VERBATIM
    )

    set_source_files_properties(
        "${_generated_header}"
        "${_generated_source}"
        "${_generated_artifact}"
        PROPERTIES GENERATED TRUE
    )
    target_sources("${TARGET_NAME}" PRIVATE "${_generated_source}")
    target_include_directories("${TARGET_NAME}" PRIVATE "${_output_directory}")
    if(NEOWPF_XAML_PUBLIC_HEADER)
        target_include_directories("${TARGET_NAME}" PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/generated/xaml")
    endif()
endfunction()
