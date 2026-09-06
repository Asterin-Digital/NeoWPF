function(neowpf_compile_xaml TARGET_NAME INPUT_XAML)
    set(options)
    set(oneValueArgs NAMESPACE COMPONENT)
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
    if(NOT TARGET NeoWPF.XamlGenerator)
        message(FATAL_ERROR
            "neowpf_compile_xaml: NeoWPF.XamlGenerator target is required")
    endif()

    get_filename_component(_xaml_name "${INPUT_XAML}" NAME_WE)
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

    add_custom_command(
        OUTPUT
            "${_generated_header}"
            "${_generated_source}"
            "${_generated_artifact}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_output_directory}"
        COMMAND "$<TARGET_FILE:NeoWPF.XamlGenerator>"
            "${INPUT_XAML}"
            "${_output_directory}"
            "${NEOWPF_XAML_NAMESPACE}"
            "${NEOWPF_XAML_COMPONENT}"
        DEPENDS
            NeoWPF.XamlGenerator
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
endfunction()
