if(UNIX)
    set(NEOWPF_RELEASE_DIRECTORY "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/$<CONFIG>")
    set(NEOWPF_RELEASE_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/Debug")
    set(NEOWPF_RELEASE_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/Release")
    set(NEOWPF_RELEASE_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/RelWithDebInfo")
    set(NEOWPF_RELEASE_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/NWpfBuild/${NEOWPF_PLATFORM}/MinSizeRel")

    set(NEOWPF_WINDOWS_RELEASE_DIRECTORY "${NEOWPF_RELEASE_DIRECTORY}")
    set(NEOWPF_WINDOWS_RELEASE_DIRECTORY_DEBUG "${NEOWPF_RELEASE_DIRECTORY_DEBUG}")
    set(NEOWPF_WINDOWS_RELEASE_DIRECTORY_RELEASE "${NEOWPF_RELEASE_DIRECTORY_RELEASE}")
    set(NEOWPF_WINDOWS_RELEASE_DIRECTORY_RELWITHDEBINFO "${NEOWPF_RELEASE_DIRECTORY_RELWITHDEBINFO}")
    set(NEOWPF_WINDOWS_RELEASE_DIRECTORY_MINSIZEREL "${NEOWPF_RELEASE_DIRECTORY_MINSIZEREL}")

    get_filename_component(_neowpf_parent_dir "${CMAKE_SOURCE_DIR}" DIRECTORY)
    file(GLOB _neowpf_mirror_candidates
        LIST_DIRECTORIES true
        "${_neowpf_parent_dir}/*/NeoWPF"
        "${_neowpf_parent_dir}/NeoWPF"
    )
    foreach(_neowpf_mirror_candidate IN LISTS _neowpf_mirror_candidates)
        if(NOT "${_neowpf_mirror_candidate}" STREQUAL "${CMAKE_SOURCE_DIR}" AND
           EXISTS "${_neowpf_mirror_candidate}/NeoWPFVersion.txt")
            set(NEOWPF_WINDOWS_RELEASE_DIRECTORY "${_neowpf_mirror_candidate}/NWpfBuild/${NEOWPF_PLATFORM}/$<CONFIG>")
            set(NEOWPF_WINDOWS_RELEASE_DIRECTORY_DEBUG "${_neowpf_mirror_candidate}/NWpfBuild/${NEOWPF_PLATFORM}/Debug")
            set(NEOWPF_WINDOWS_RELEASE_DIRECTORY_RELEASE "${_neowpf_mirror_candidate}/NWpfBuild/${NEOWPF_PLATFORM}/Release")
            set(NEOWPF_WINDOWS_RELEASE_DIRECTORY_RELWITHDEBINFO "${_neowpf_mirror_candidate}/NWpfBuild/${NEOWPF_PLATFORM}/RelWithDebInfo")
            set(NEOWPF_WINDOWS_RELEASE_DIRECTORY_MINSIZEREL "${_neowpf_mirror_candidate}/NWpfBuild/${NEOWPF_PLATFORM}/MinSizeRel")
            break()
        endif()
    endforeach()
    unset(_neowpf_parent_dir)
    unset(_neowpf_mirror_candidates)
    unset(_neowpf_mirror_candidate)
endif()

if(NEOWPF_STATIC_DEPENDENCIES)
    set(NEOWPF_STATIC_DEPENDENCY_TARGETS
        CURL::libcurl
        MbedTLS::mbedcrypto
        MbedTLS::mbedx509
        MbedTLS::mbedtls
        OpenSSL::Crypto
        OpenSSL::SSL
        ZLIB::ZLIB
        PNG::PNG
        Vulkan::Vulkan
        PkgConfig::WAYLAND
        PkgConfig::XKBCOMMON
        Freetype::Freetype
        PkgConfig::HARFBUZZ
        PkgConfig::FONTCONFIG
    )
    set(NEOWPF_STATIC_ARCHIVES_DEBUG)
    set(NEOWPF_STATIC_ARCHIVES_RELEASE)
    foreach(NEOWPF_DEPENDENCY_TARGET IN LISTS NEOWPF_STATIC_DEPENDENCY_TARGETS)
        if(NOT TARGET "${NEOWPF_DEPENDENCY_TARGET}")
            continue()
        endif()

        get_target_property(NEOWPF_DEPENDENCY_DEBUG
            "${NEOWPF_DEPENDENCY_TARGET}" IMPORTED_LOCATION_DEBUG)
        get_target_property(NEOWPF_DEPENDENCY_RELEASE
            "${NEOWPF_DEPENDENCY_TARGET}" IMPORTED_LOCATION_RELEASE)
        get_target_property(NEOWPF_DEPENDENCY_LOCATION
            "${NEOWPF_DEPENDENCY_TARGET}" IMPORTED_LOCATION)
        if(NOT NEOWPF_DEPENDENCY_DEBUG)
            set(NEOWPF_DEPENDENCY_DEBUG "${NEOWPF_DEPENDENCY_LOCATION}")
        endif()
        if(NOT NEOWPF_DEPENDENCY_RELEASE)
            set(NEOWPF_DEPENDENCY_RELEASE "${NEOWPF_DEPENDENCY_LOCATION}")
        endif()

        if(NEOWPF_DEPENDENCY_DEBUG MATCHES "\\.(a|lib)$")
            list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG "${NEOWPF_DEPENDENCY_DEBUG}")
        endif()
        if(NEOWPF_DEPENDENCY_RELEASE MATCHES "\\.(a|lib)$")
            list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE "${NEOWPF_DEPENDENCY_RELEASE}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_DEBUG)
    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_RELEASE)

    foreach(NEOWPF_STATIC_LIBRARY_SPEC IN ITEMS
        "wayland-client;${WAYLAND_LIBRARY_DIRS}"
        "xkbcommon;${XKBCOMMON_LIBRARY_DIRS}"
        "freetype;${FREETYPE_LIBRARY_DIRS}"
        "harfbuzz;${HARFBUZZ_LIBRARY_DIRS}"
        "fontconfig;${FONTCONFIG_LIBRARY_DIRS}")
        list(GET NEOWPF_STATIC_LIBRARY_SPEC 0 NEOWPF_STATIC_LIBRARY_NAME)
        list(REMOVE_AT NEOWPF_STATIC_LIBRARY_SPEC 0)
        find_file(NEOWPF_STATIC_LIBRARY
            "lib${NEOWPF_STATIC_LIBRARY_NAME}.a"
            PATHS ${NEOWPF_STATIC_LIBRARY_SPEC}
            NO_DEFAULT_PATH)
        if(NEOWPF_STATIC_LIBRARY)
            list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG "${NEOWPF_STATIC_LIBRARY}")
            list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE "${NEOWPF_STATIC_LIBRARY}")
        endif()
        unset(NEOWPF_STATIC_LIBRARY CACHE)
    endforeach()

    set(NEOWPF_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    foreach(NEOWPF_TRANSITIVE_LIBRARY_NAME IN ITEMS
        bz2
        z
        png16
        brotlidec
        brotlicommon
        expat)
        find_library(NEOWPF_TRANSITIVE_LIBRARY
            NAMES "${NEOWPF_TRANSITIVE_LIBRARY_NAME}")
        if(NEOWPF_TRANSITIVE_LIBRARY)
            list(APPEND NEOWPF_STATIC_ARCHIVES_DEBUG "${NEOWPF_TRANSITIVE_LIBRARY}")
            list(APPEND NEOWPF_STATIC_ARCHIVES_RELEASE "${NEOWPF_TRANSITIVE_LIBRARY}")
        endif()
        unset(NEOWPF_TRANSITIVE_LIBRARY CACHE)
    endforeach()
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${NEOWPF_FIND_LIBRARY_SUFFIXES}")
    unset(NEOWPF_FIND_LIBRARY_SUFFIXES)

    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_DEBUG)
    list(REMOVE_DUPLICATES NEOWPF_STATIC_ARCHIVES_RELEASE)

    if(WIN32)
        set(NEOWPF_WINDOWS_SYSTEM_LIBRARIES "")
        foreach(NEOWPF_SYSTEM_LIBRARY_NAME IN ITEMS
            d3d11
            dxgi
            dbghelp
            ole32
            windowscodecs
            bcrypt
            crypt32
            ws2_32
            wldap32
            winmm
            advapi32
            iphlpapi
            normaliz
            secur32
            dwmapi)
            find_library(NEOWPF_SYSTEM_LIBRARY
                NAMES "${NEOWPF_SYSTEM_LIBRARY_NAME}")
            if(NEOWPF_SYSTEM_LIBRARY)
                list(APPEND NEOWPF_WINDOWS_SYSTEM_LIBRARIES "${NEOWPF_SYSTEM_LIBRARY}")
            endif()
            unset(NEOWPF_SYSTEM_LIBRARY CACHE)
        endforeach()

        set(NEOWPF_WINDOWS_MERGE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/merge-neowpf-libraries-$<CONFIG>.cmake")
        string(CONCAT NEOWPF_WINDOWS_MERGE_SCRIPT_CONTENT
            "set(NEOWPF_ARCHIVER [==[${CMAKE_AR}]==])\n"
            "set(NEOWPF_TARGET [==[$<TARGET_FILE:NeoWPF>]==])\n"
            "set(NEOWPF_TRIPLET [==[${NEOWPF_VCPKG_TRIPLET}]==])\n"
            "set(NEOWPF_SYSTEM_LIBRARIES [==[${NEOWPF_WINDOWS_SYSTEM_LIBRARIES}]==])\n"
            "if(\"$<CONFIG>\" STREQUAL \"Debug\")\n"
            "    set(NEOWPF_LIBRARY_DIR \"\${NEOWPF_TRIPLET}/debug/lib\")\n"
            "else()\n"
            "    set(NEOWPF_LIBRARY_DIR \"\${NEOWPF_TRIPLET}/lib\")\n"
            "endif()\n"
            "file(GLOB_RECURSE NEOWPF_DEPENDENCY_LIBRARIES \"\${NEOWPF_LIBRARY_DIR}/*.lib\")\n"
            "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy \"\${NEOWPF_TARGET}\" \"\${NEOWPF_TARGET}.base\" RESULT_VARIABLE NEOWPF_COPY_RESULT)\n"
            "if(NEOWPF_COPY_RESULT)\n"
            "    message(FATAL_ERROR \"NeoWPF archive staging failed with exit code \${NEOWPF_COPY_RESULT}\")\n"
            "endif()\n"
            "execute_process(COMMAND \"\${NEOWPF_ARCHIVER}\" \"/OUT:\${NEOWPF_TARGET}\" \"/IGNORE:4006\" \"\${NEOWPF_TARGET}.base\" \${NEOWPF_DEPENDENCY_LIBRARIES} \${NEOWPF_SYSTEM_LIBRARIES} RESULT_VARIABLE NEOWPF_ARCHIVE_RESULT)\n"
            "file(REMOVE \"\${NEOWPF_TARGET}.base\")\n"
            "if(NEOWPF_ARCHIVE_RESULT)\n"
            "    message(FATAL_ERROR \"NeoWPF dependency merge failed with exit code \${NEOWPF_ARCHIVE_RESULT}\")\n"
            "endif()\n"
        )
        file(GENERATE OUTPUT "${NEOWPF_WINDOWS_MERGE_SCRIPT}" CONTENT
            "${NEOWPF_WINDOWS_MERGE_SCRIPT_CONTENT}"
        )
        add_custom_command(TARGET NeoWPF POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -P "${NEOWPF_WINDOWS_MERGE_SCRIPT}"
            VERBATIM
        )
    elseif(UNIX)
        set(NEOWPF_RELEASE_ARCHIVE
            "${NEOWPF_RELEASE_DIRECTORY}/NeoWPF.${NEOWPF_PLATFORM}.$<CONFIG>.${PROJECT_VERSION}.a")
        set(NEOWPF_WINDOWS_RELEASE_ARCHIVE
            "${NEOWPF_WINDOWS_RELEASE_DIRECTORY}/NeoWPF.${NEOWPF_PLATFORM}.$<CONFIG>.${PROJECT_VERSION}.a")
        set(NEOWPF_ARCHIVE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/merge-neowpf-archives-$<CONFIG>.mri")
        set(NEOWPF_ARCHIVE_SCRIPT_CONTENT
            "create ${NEOWPF_RELEASE_ARCHIVE}\n"
            "addlib $<TARGET_FILE:NeoWPF>\n"
        )
        foreach(NEOWPF_STATIC_ARCHIVE IN LISTS NEOWPF_STATIC_ARCHIVES_RELEASE)
            string(APPEND NEOWPF_ARCHIVE_SCRIPT_CONTENT "addlib ${NEOWPF_STATIC_ARCHIVE}\n")
        endforeach()
        string(APPEND NEOWPF_ARCHIVE_SCRIPT_CONTENT "save\nend\n")

        file(GENERATE OUTPUT "${NEOWPF_ARCHIVE_SCRIPT}" CONTENT "${NEOWPF_ARCHIVE_SCRIPT_CONTENT}")

        add_custom_command(TARGET NeoWPF POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_RELEASE_DIRECTORY}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${NEOWPF_WINDOWS_RELEASE_DIRECTORY}"
            COMMAND sh -c "\"${CMAKE_AR}\" -M < \"${NEOWPF_ARCHIVE_SCRIPT}\""
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${NEOWPF_RELEASE_ARCHIVE}"
                "${NEOWPF_WINDOWS_RELEASE_ARCHIVE}"
            VERBATIM
        )
    endif()
endif()
