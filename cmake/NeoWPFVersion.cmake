function(neowpf_read_version_file version_file output_variable)
    if(NOT EXISTS "${version_file}")
        message(FATAL_ERROR "Could not locate NeoWPF version file: ${version_file}")
    endif()

    file(READ "${version_file}" _neowpf_version)
    string(STRIP "${_neowpf_version}" _neowpf_version)
    if(NOT _neowpf_version MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+\\.([0-9]+)$")
        message(FATAL_ERROR
            "${version_file} must contain Major.Minor.Patch.Build")
    endif()
    set(_neowpf_build_number "${CMAKE_MATCH_1}")
    if(_neowpf_build_number GREATER 4294967295)
        message(FATAL_ERROR
            "${version_file} Build component must fit in a 32-bit unsigned integer")
    endif()

    set(${output_variable} "${_neowpf_version}" PARENT_SCOPE)
endfunction()

function(neowpf_resolve_version_file neowpf_root output_variable)
    if(DEFINED NEOWPF_SHARED_VERSION_FILE AND
       NOT "${NEOWPF_SHARED_VERSION_FILE}" STREQUAL "")
        set(_neowpf_version_file "${NEOWPF_SHARED_VERSION_FILE}")
    elseif(DEFINED ENV{NEOWPF_SHARED_VERSION_FILE} AND
           NOT "$ENV{NEOWPF_SHARED_VERSION_FILE}" STREQUAL "")
        set(_neowpf_version_file "$ENV{NEOWPF_SHARED_VERSION_FILE}")
    else()
        set(_neowpf_version_file "${neowpf_root}/NeoWPFVersion.txt")
        set(_neowpf_is_wsl FALSE)
        if(DEFINED ENV{WSL_DISTRO_NAME})
            set(_neowpf_is_wsl TRUE)
        elseif(EXISTS "/proc/version")
            file(READ "/proc/version" _neowpf_kernel_version)
            if(_neowpf_kernel_version MATCHES "[Mm]icrosoft")
                set(_neowpf_is_wsl TRUE)
            endif()
        endif()

        if(_neowpf_is_wsl AND "${neowpf_root}" MATCHES "^/home/[^/]+/\\.vs/")
            file(GLOB _neowpf_shared_version_candidates
                "/mnt/c/Users/*/source/repos/NeoWPF/NeoWPFVersion.txt")
            list(LENGTH _neowpf_shared_version_candidates _neowpf_candidate_count)
            if(_neowpf_candidate_count EQUAL 1)
                list(GET _neowpf_shared_version_candidates 0 _neowpf_version_file)
            else()
                message(FATAL_ERROR
                    "Could not uniquely locate the shared Windows NeoWPFVersion.txt. "
                    "Set NEOWPF_SHARED_VERSION_FILE to its /mnt/c/... path.")
            endif()
        endif()
    endif()

    if(NOT EXISTS "${_neowpf_version_file}")
        message(FATAL_ERROR "Could not locate NeoWPF version file: ${_neowpf_version_file}")
    endif()
    set(${output_variable} "${_neowpf_version_file}" PARENT_SCOPE)
endfunction()

function(neowpf_read_artifact_version neowpf_root output_variable)
    neowpf_resolve_version_file("${neowpf_root}" _neowpf_source_version_file)
    neowpf_read_version_file("${_neowpf_source_version_file}" _neowpf_source_version)
    set(_neowpf_selected_version "${_neowpf_source_version}")

    set(_neowpf_built_version_file "${neowpf_root}/NWpfBuild/NeoWPFBuiltVersion.txt")
    if(EXISTS "${_neowpf_built_version_file}")
        neowpf_read_version_file("${_neowpf_built_version_file}" _neowpf_built_version)
        string(REGEX REPLACE "\\.[0-9]+$" "" _neowpf_source_base "${_neowpf_source_version}")
        string(REGEX REPLACE "\\.[0-9]+$" "" _neowpf_built_base "${_neowpf_built_version}")
        string(REGEX MATCH "([0-9]+)$" _neowpf_source_build_match "${_neowpf_source_version}")
        set(_neowpf_source_build "${CMAKE_MATCH_1}")
        string(REGEX MATCH "([0-9]+)$" _neowpf_built_build_match "${_neowpf_built_version}")
        set(_neowpf_built_build "${CMAKE_MATCH_1}")

        if(_neowpf_source_base STREQUAL _neowpf_built_base AND
           _neowpf_built_build LESS_EQUAL _neowpf_source_build)
            set(_neowpf_selected_version "${_neowpf_built_version}")
        endif()
    endif()

    set(${output_variable} "${_neowpf_selected_version}" PARENT_SCOPE)
endfunction()