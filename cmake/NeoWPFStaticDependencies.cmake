## Dear CMake devs
## I hate all of you!!! i went through 5 hours of debugging just for static lib merging D:

if(NEOWPF_STATIC_DEPENDENCIES)
    if(WIN32)
        find_program(NEOWPF_LIB_TOOL NAMES lib.exe lib)
        if(NOT NEOWPF_LIB_TOOL)
            message(FATAL_ERROR "Could not find Microsoft lib.exe for static archive merging.")
        endif()
        message(STATUS "[NeoWPF static deps] Using archive merger: ${NEOWPF_LIB_TOOL}")
    else()
        if(CMAKE_AR AND EXISTS "${CMAKE_AR}")
            set(NEOWPF_AR_TOOL "${CMAKE_AR}")
        else()
            find_program(NEOWPF_AR_TOOL NAMES ar llvm-ar)
        endif()
        if(NOT NEOWPF_AR_TOOL)
            message(FATAL_ERROR "Could not find ar or llvm-ar for static archive merging.")
        endif()
        message(STATUS "[NeoWPF static deps] Using archive merger: ${NEOWPF_AR_TOOL}")
    endif()

    set(NEOWPF_STATIC_ARCHIVES_DEBUG)
    set(NEOWPF_STATIC_ARCHIVES_RELEASE)

    function(neowpf_resolve_linked_archive _link_name _out_var)
        set(_resolved "")

        if(TARGET "${_link_name}")
            get_target_property(_target_debug_location "${_link_name}" IMPORTED_LOCATION_DEBUG)
            get_target_property(_target_release_location "${_link_name}" IMPORTED_LOCATION_RELEASE)
            get_target_property(_target_default_location "${_link_name}" IMPORTED_LOCATION)

            if(_target_debug_location AND _target_debug_location MATCHES "\\.(a|lib)$")
                set(_resolved "${_target_debug_location}")
            elseif(_target_release_location AND _target_release_location MATCHES "\\.(a|lib)$")
                set(_resolved "${_target_release_location}")
            elseif(_target_default_location AND _target_default_location MATCHES "\\.(a|lib)$")
                set(_resolved "${_target_default_location}")
            endif()
        elseif(_link_name MATCHES "[/\\]")
            set(_resolved "${_link_name}")
        else()
            set(_candidate_names "${_link_name}")
            if(NOT _link_name MATCHES "\\.(a|lib)$")
                if(WIN32)
                    list(APPEND _candidate_names "${_link_name}.lib")
                else()
                    list(APPEND _candidate_names "${_link_name}.a")
                endif()
            endif()

            unset(_resolved_path CACHE)
            set(_neowpf_saved_find_library_suffixes "${CMAKE_FIND_LIBRARY_SUFFIXES}")
            if(WIN32)
                set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
            else()
                set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
            endif()
            find_library(_resolved_path NAMES ${_candidate_names})
            set(CMAKE_FIND_LIBRARY_SUFFIXES "${_neowpf_saved_find_library_suffixes}")
            unset(_neowpf_saved_find_library_suffixes)
            if(NOT _resolved_path)
                find_library(_resolved_path NAMES ${_candidate_names})
            endif()
            if(_resolved_path)
                set(_resolved "${_resolved_path}")
            endif()
            unset(_resolved_path CACHE)

            if(NOT _resolved AND WIN32)
                set(_neowpf_windows_sdk_dirs)
                if(DEFINED ENV{WindowsSdkDir})
                    list(APPEND _neowpf_windows_sdk_dirs
                        "$ENV{WindowsSdkDir}/Lib"
                    )
                endif()

                if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
                    foreach(_sdk_ver IN ITEMS "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
                        if(DEFINED ENV{WindowsSdkDir})
                            list(APPEND _neowpf_windows_sdk_dirs
                                "$ENV{WindowsSdkDir}/Lib/${_sdk_ver}/um/x64"
                                "$ENV{WindowsSdkDir}/Lib/${_sdk_ver}/um/x86"
                            )
                        endif()
                    endforeach()
                endif()

                if(CMAKE_SYSTEM_VERSION)
                    foreach(_sdk_ver IN ITEMS "${CMAKE_SYSTEM_VERSION}")
                        if(DEFINED ENV{WindowsSdkDir})
                            list(APPEND _neowpf_windows_sdk_dirs
                                "$ENV{WindowsSdkDir}/Lib/${_sdk_ver}/um/x64"
                                "$ENV{WindowsSdkDir}/Lib/${_sdk_ver}/um/x86"
                            )
                        endif()
                    endforeach()
                endif()

                list(REMOVE_DUPLICATES _neowpf_windows_sdk_dirs)
                foreach(_sdk_lib_name IN ITEMS
                    d3d11 dxgi dcomp dbghelp ole32 windowscodecs bcrypt crypt32 ws2_32 wldap32 winmm advapi32 iphlpapi normaliz secur32 dwmapi)
                    if("${_link_name}" STREQUAL "${_sdk_lib_name}")
                        foreach(_sdk_dir IN LISTS _neowpf_windows_sdk_dirs)
                            if(EXISTS "${_sdk_dir}/${_sdk_lib_name}.lib")
                                set(_resolved "${_sdk_dir}/${_sdk_lib_name}.lib")
                                break()
                            endif()
                        endforeach()
                    endif()
                endforeach()
            endif()
        endif()

        if(_resolved AND _resolved MATCHES "\\.(a|lib)$")
            set(${_out_var} "${_resolved}" PARENT_SCOPE)
        else()
            unset(${_out_var} PARENT_SCOPE)
        endif()
    endfunction()

    function(neowpf_collect_static_archives _root_target _output_var)
        set(_visited)
        set(_queue "${_root_target}")
        set(_archives_debug)
        set(_archives_release)

        while(_queue)
            list(POP_FRONT _queue _current_target)
            if(NOT _current_target)
                continue()
            endif()

            list(FIND _visited "${_current_target}" _visited_index)
            if(NOT _visited_index EQUAL -1)
                continue()
            endif()
            list(APPEND _visited "${_current_target}")

            get_target_property(_current_type "${_current_target}" TYPE)
            if(_current_type STREQUAL "STATIC_LIBRARY" OR _current_type STREQUAL "UNKNOWN_LIBRARY")
                get_target_property(_current_debug_location "${_current_target}" IMPORTED_LOCATION_DEBUG)
                get_target_property(_current_release_location "${_current_target}" IMPORTED_LOCATION_RELEASE)
                get_target_property(_current_default_location "${_current_target}" IMPORTED_LOCATION)

                if(NOT _current_debug_location)
                    set(_current_debug_location "${_current_default_location}")
                endif()
                if(NOT _current_release_location)
                    set(_current_release_location "${_current_default_location}")
                endif()

                if(_current_debug_location AND _current_debug_location MATCHES "\\.(a|lib)$")
                    list(APPEND _archives_debug "${_current_debug_location}")
                endif()
                if(_current_release_location AND _current_release_location MATCHES "\\.(a|lib)$")
                    list(APPEND _archives_release "${_current_release_location}")
                endif()
            endif()

            get_target_property(_current_link_libraries "${_current_target}" LINK_LIBRARIES)
            get_target_property(_current_interface_link_libraries "${_current_target}" INTERFACE_LINK_LIBRARIES)
            get_target_property(_current_imported_dependent_libraries "${_current_target}" IMPORTED_LINK_DEPENDENT_LIBRARIES)

            foreach(_linked_target IN LISTS
                    _current_link_libraries
                    _current_interface_link_libraries
                    _current_imported_dependent_libraries)
                if(NOT _linked_target)
                    continue()
                endif()

                string(REPLACE "$<LINK_ONLY:" "" _linked_target "${_linked_target}")
                string(REPLACE ">" "" _linked_target "${_linked_target}")
                string(STRIP "${_linked_target}" _linked_target)

                if(TARGET "${_linked_target}")
                    list(APPEND _queue "${_linked_target}")
                    continue()
                endif()

                neowpf_resolve_linked_archive("${_linked_target}" _linked_archive)
                if(_linked_archive)
                    message(STATUS "[NeoWPF static deps] Resolved linked archive for ${_linked_target}: ${_linked_archive}")
                    list(APPEND _archives_release "${_linked_archive}")
                    list(APPEND _archives_debug "${_linked_archive}")
                endif()
            endforeach()
        endwhile()

        list(REMOVE_DUPLICATES _archives_debug)
        list(REMOVE_DUPLICATES _archives_release)
        set(${_output_var}_DEBUG "${_archives_debug}" PARENT_SCOPE)
        set(${_output_var}_RELEASE "${_archives_release}" PARENT_SCOPE)
    endfunction()

    foreach(_neowpf_target_name IN ITEMS NeoWPF NeoWPF.Foundation)
        if(NOT TARGET "${_neowpf_target_name}")
            continue()
        endif()

        neowpf_collect_static_archives("${_neowpf_target_name}" _neowpf_archives)
        list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG ${_neowpf_archives_DEBUG})
        list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE ${_neowpf_archives_RELEASE})
    endforeach()

    if(TARGET Cudev)
        list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG "$<TARGET_FILE:Cudev>")
        list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE "$<TARGET_FILE:Cudev>")
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND TARGET ZLIB::ZLIB)
        neowpf_resolve_linked_archive("ZLIB::ZLIB" _neowpf_zlib_archive)
        if(_neowpf_zlib_archive)
            list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG "${_neowpf_zlib_archive}")
            list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE "${_neowpf_zlib_archive}")
        endif()
    endif()

    if(WIN32)
        set(NEOWPF_WINDOWS_SDK_STATIC_LIBS
            d3d11 dxgi dcomp dbghelp ole32 windowscodecs bcrypt crypt32 ws2_32
            wldap32 winmm advapi32 iphlpapi normaliz secur32 dwmapi)

        set(_neowpf_windows_sdk_dirs)
        if(DEFINED ENV{NEOWPF_WINDOWS_SDK_ROOT})
            list(APPEND _neowpf_windows_sdk_dirs "$ENV{NEOWPF_WINDOWS_SDK_ROOT}")
            list(APPEND _neowpf_windows_sdk_dirs "$ENV{NEOWPF_WINDOWS_SDK_ROOT}/um/x64")
            list(APPEND _neowpf_windows_sdk_dirs "$ENV{NEOWPF_WINDOWS_SDK_ROOT}/um/x86")
            list(APPEND _neowpf_windows_sdk_dirs "$ENV{NEOWPF_WINDOWS_SDK_ROOT}/../")
            message(STATUS "[NeoWPF static deps] Using NEOWPF_WINDOWS_SDK_ROOT: $ENV{NEOWPF_WINDOWS_SDK_ROOT}")
        else()
            message(FATAL_ERROR "NEOWPF_WINDOWS_SDK_ROOT is required when NEOWPF_STATIC_DEPENDENCIES=ON.")
        endif()
        list(REMOVE_DUPLICATES _neowpf_windows_sdk_dirs)

        foreach(_neowpf_sdk_lib IN LISTS NEOWPF_WINDOWS_SDK_STATIC_LIBS)
            set(_neowpf_sdk_archive "")
            foreach(_neowpf_sdk_dir IN LISTS _neowpf_windows_sdk_dirs)
                if(EXISTS "${_neowpf_sdk_dir}/${_neowpf_sdk_lib}.lib")
                    set(_neowpf_sdk_archive "${_neowpf_sdk_dir}/${_neowpf_sdk_lib}.lib")
                    break()
                endif()
            endforeach()
            if(_neowpf_sdk_archive)
                message(STATUS "[NeoWPF static deps] Added Windows SDK archive: ${_neowpf_sdk_lib} -> ${_neowpf_sdk_archive}")
                list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG "${_neowpf_sdk_archive}")
                list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE "${_neowpf_sdk_archive}")
            else()
                message(STATUS "[NeoWPF static deps] Windows SDK archive not found: ${_neowpf_sdk_lib}")
            endif()
        endforeach()
    endif()

    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_DEBUG)
    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_RELEASE)
    message(STATUS "[NeoWPF static deps] Final debug archive set: ${NEOWPF_STATIC_ARCHIVES_DEBUG}")
    message(STATUS "[NeoWPF static deps] Final release archive set: ${NEOWPF_STATIC_ARCHIVES_RELEASE}")

    if(NOT NEOWPF_STATIC_ARCHIVES_DEBUG AND NOT NEOWPF_STATIC_ARCHIVES_RELEASE)
        message(FATAL_ERROR "NEOWPF_STATIC_DEPENDENCIES is enabled but no static archive dependencies were found in the actual NeoWPF link graph.")
    endif()

    function(neowpf_add_static_dependency_merge _target_name)
        cmake_parse_arguments(NEOWPF_MERGE "EXTRA_ONLY" "" "" ${ARGN})
        set(_extra_static_archives ${NEOWPF_MERGE_UNPARSED_ARGUMENTS})

        set(_merge_script "${CMAKE_CURRENT_BINARY_DIR}/merge-${_target_name}-$<CONFIG>.cmake")
        set(_merge_stamp "${CMAKE_CURRENT_BINARY_DIR}/.merge-${_target_name}-$<CONFIG>.stamp")
        if(NEOWPF_MERGE_EXTRA_ONLY)
            set(_merge_static_archives_debug)
            set(_merge_static_archives_release)
        else()
            set(_merge_static_archives_debug "${NEOWPF_STATIC_ARCHIVES_DEBUG}")
            set(_merge_static_archives_release "${NEOWPF_STATIC_ARCHIVES_RELEASE}")
        endif()
        string(CONCAT _merge_script_content
            "set(NEOWPF_TARGET [==[$<TARGET_FILE:${_target_name}>]==])\n"
            "set(NEOWPF_STATIC_ARCHIVES_DEBUG [==[${_merge_static_archives_debug}]==])\n"
            "set(NEOWPF_STATIC_ARCHIVES_RELEASE [==[${_merge_static_archives_release}]==])\n"
            "set(NEOWPF_EXTRA_ARCHIVES [==[${_extra_static_archives}]==])\n"
            "set(NEOWPF_STAGED_TARGET \"\${NEOWPF_TARGET}.base\")\n"
            "if(\"$<CONFIG>\" STREQUAL \"Debug\")\n"
            "    set(NEOWPF_INPUT_ARCHIVES \"\${NEOWPF_STATIC_ARCHIVES_DEBUG}\")\n"
            "else()\n"
            "    set(NEOWPF_INPUT_ARCHIVES \"\${NEOWPF_STATIC_ARCHIVES_RELEASE}\")\n"
            "endif()\n"
            "if(NOT NEOWPF_INPUT_ARCHIVES AND NOT NEOWPF_EXTRA_ARCHIVES)\n"
            "    message(FATAL_ERROR \"No static archives were available for ${_target_name} in configuration $<CONFIG>.\")\n"
            "endif()\n"
            "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy \"\${NEOWPF_TARGET}\" \"\${NEOWPF_STAGED_TARGET}\" RESULT_VARIABLE NEOWPF_COPY_RESULT)\n"
            "if(NEOWPF_COPY_RESULT)\n"
            "    message(FATAL_ERROR \"${_target_name} archive staging failed with exit code \${NEOWPF_COPY_RESULT}\")\n"
            "endif()\n"
            "set(NEOWPF_ARCHIVE_INPUTS \"\${NEOWPF_STAGED_TARGET}\")\n"
            "list(APPEND NEOWPF_ARCHIVE_INPUTS \${NEOWPF_INPUT_ARCHIVES})\n"
            "list(APPEND NEOWPF_ARCHIVE_INPUTS \${NEOWPF_EXTRA_ARCHIVES})\n"
        )
        if(WIN32)
            string(APPEND _merge_script_content
                "execute_process(COMMAND \"${NEOWPF_LIB_TOOL}\" /NOLOGO /IGNORE:4006 \"/OUT:\${NEOWPF_TARGET}\" \${NEOWPF_ARCHIVE_INPUTS} RESULT_VARIABLE NEOWPF_ARCHIVE_RESULT)\n"
            )
        else()
            string(APPEND _merge_script_content
                "set(NEOWPF_AR_SCRIPT \"\${NEOWPF_TARGET}.mri\")\n"
                "file(WRITE \"\${NEOWPF_AR_SCRIPT}\" \"CREATE \${NEOWPF_TARGET}\\n\")\n"
                "foreach(NEOWPF_ARCHIVE IN LISTS NEOWPF_ARCHIVE_INPUTS)\n"
                "    file(APPEND \"\${NEOWPF_AR_SCRIPT}\" \"ADDLIB \${NEOWPF_ARCHIVE}\\n\")\n"
                "endforeach()\n"
                "file(APPEND \"\${NEOWPF_AR_SCRIPT}\" \"SAVE\\nEND\\n\")\n"
                "execute_process(COMMAND \"${NEOWPF_AR_TOOL}\" -M INPUT_FILE \"\${NEOWPF_AR_SCRIPT}\" RESULT_VARIABLE NEOWPF_ARCHIVE_RESULT)\n"
                "file(REMOVE \"\${NEOWPF_AR_SCRIPT}\")\n"
            )
        endif()
        string(APPEND _merge_script_content
            "file(REMOVE \"\${NEOWPF_STAGED_TARGET}\")\n"
            "if(NEOWPF_ARCHIVE_RESULT)\n"
            "    message(FATAL_ERROR \"${_target_name} static dependency merge failed with exit code \${NEOWPF_ARCHIVE_RESULT}\")\n"
            "endif()\n"
            "file(WRITE \"${_merge_stamp}\" \"${_target_name}:$<CONFIG>\\n\")\n"
        )
        file(GENERATE OUTPUT "${_merge_script}" CONTENT "${_merge_script_content}")

        add_custom_command(
            OUTPUT "${_merge_stamp}"
            COMMAND "${CMAKE_COMMAND}" -P "${_merge_script}"
            DEPENDS "${_merge_script}" "${CMAKE_CURRENT_LIST_FILE}" "${CMAKE_SOURCE_DIR}/CMakeLists.txt" "${_target_name}"
            VERBATIM
            COMMENT "Merging static dependencies for ${_target_name}"
        )

        add_custom_command(TARGET ${_target_name} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -P "${_merge_script}"
            VERBATIM
            COMMENT "Merging static dependencies for ${_target_name}"
        )
    endfunction()

    if(TARGET NeoWPF)
        neowpf_add_static_dependency_merge(NeoWPF)
    endif()

    if(NEOWPF_BUILD_MEDIA AND TARGET NeoWPF.Media)
        neowpf_add_static_dependency_merge(NeoWPF.Media $<TARGET_FILE:LuAudio>)
    endif()
endif()