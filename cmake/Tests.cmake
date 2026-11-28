if(BUILD_TESTING AND NOT ANDROID)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
    )

    if(MSVC)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    FetchContent_MakeAvailable(googletest)

    add_executable(NeoWPF.AutomatedTests
        tests/Automated/AndroidNativeActivityTests.cpp
        tests/Automated/EntryRunnerTests.cpp
        tests/Automated/ApplicationTests.cpp
        tests/Automated/PlatformRegistryTests.cpp
        tests/Automated/ApplicationHostTests.cpp
        tests/Automated/ApplicationWindowSessionTests.cpp
        tests/Automated/ApplicationResourcesTests.cpp
        tests/Automated/ApplicationStorageTests.cpp
        tests/Automated/EmbeddedResourceProviderTests.cpp
        tests/Automated/ResourceKeyTests.cpp
        tests/Automated/ExternalFileResourceProviderTests.cpp
        tests/Automated/NamlArtifactTests.cpp
        tests/Automated/NotificationTests.cpp
        tests/Automated/LocalizationTests.cpp
        tests/Automated/ApplicationThemeTests.cpp
        tests/Automated/EventTests.cpp
        tests/Automated/RoutedEventTests.cpp
        tests/Automated/DependencyPropertyTests.cpp
        tests/Automated/ResultTests.cpp
        tests/Automated/DiagnosticsTests.cpp
        tests/Automated/BrushTests.cpp
        tests/Automated/BrushPaintResolutionTests.cpp
        tests/Automated/WindowBackgroundPaintTests.cpp
        tests/Automated/PanelPaintTests.cpp
        tests/Automated/ControlPaintTests.cpp
        tests/Automated/CheckboxTests.cpp
        tests/Automated/ToggleSwitchTests.cpp
        tests/Automated/ControlThemeTests.cpp
        tests/Automated/ButtonTests.cpp
        tests/Automated/ComboBoxTests.cpp
        tests/Automated/SliderTests.cpp
        tests/Automated/KeyboardTests.cpp
        tests/Automated/CapabilitiesTests.cpp
        tests/Automated/ThumbTests.cpp
        tests/Automated/CommonConvertersTests.cpp
        tests/Automated/FileDialogTests.cpp
        tests/Automated/TransformTests.cpp
        tests/Automated/LayoutTests.cpp
        tests/Automated/TextFlowTests.cpp
        tests/Automated/TransformCompositionTests.cpp
        tests/Automated/ArcCommandTests.cpp
        tests/Automated/RenderCommandTests.cpp
        tests/Automated/PolygonClipTests.cpp
        tests/Automated/DrawBatcherTests.cpp
        tests/Automated/PaintTests.cpp
        tests/Automated/RampBakerTests.cpp
        tests/Automated/CommandListHashTests.cpp
        tests/Automated/GridTests.cpp
        tests/Automated/PanelTests.cpp
        tests/Automated/ResponsiveLayoutTests.cpp
        tests/Automated/StackPanelTests.cpp
        tests/Automated/UniformGridTests.cpp
        tests/Automated/CanvasTests.cpp
        tests/Automated/ItemsControlTests.cpp
        tests/Automated/ListBoxTests.cpp
        tests/Automated/ContentPresenterTests.cpp
        tests/Automated/PopupTests.cpp
        tests/Automated/ProgressBarTests.cpp
        tests/Automated/ProgressRingTests.cpp
        tests/Automated/LImageDecodingTests.cpp
        tests/Automated/TriggerTests.cpp
        tests/Automated/VisualTests.cpp
        tests/Automated/GroupedToggleBaseTests.cpp
        tests/Automated/RadioButtonTests.cpp
        tests/Automated/ThreadingTests.cpp
        tests/Automated/XamlParserTests.cpp
        tests/Automated/XamlElementPropertyTests.cpp
        tests/Automated/TextBoxTests.cpp
        tests/Automated/ClippingTests.cpp
        tests/Automated/ScrollViewerTests.cpp
    )

    if(NOT ANDROID)
        target_sources(NeoWPF.AutomatedTests PRIVATE
            src/Platform/Android/App/AActivityLifecycle.cpp
        )
    endif()
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_sources(NeoWPF.AutomatedTests PRIVATE
            tests/Automated/LinuxBackendTests.cpp
        )
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_include_directories(NeoWPF.AutomatedTests SYSTEM PRIVATE
            "${NEOWPF_WAYLAND_GENERATED_DIR}"
            "${NEOWPF_VULKAN_GENERATED_DIR}"
        )
    endif()

    target_include_directories(NeoWPF.AutomatedTests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include/Internal"
    )
    target_link_libraries(NeoWPF.AutomatedTests
        PRIVATE
            NeoWPF::NeoWPF
            GTest::gtest_main
    )

    target_compile_definitions(NeoWPF.AutomatedTests PRIVATE
        NEOWPF_INTERNAL_BUILD
        NEOWPF_ENABLE_RUNTIME_XAML
    )

    include(GoogleTest)
    gtest_discover_tests(NeoWPF.AutomatedTests)

    if(TARGET NeoWPF.DesktopEntry)
        foreach(NEOWPF_ENTRY_LINK_FIXTURE IN ITEMS
            EntryDuplicateMain
            EntryMissingDefinition)
            set(NEOWPF_ENTRY_LINK_TARGET "NeoWPF.${NEOWPF_ENTRY_LINK_FIXTURE}")
            if(NEOWPF_ENTRY_LINK_FIXTURE STREQUAL "EntryDuplicateMain")
                set(NEOWPF_ENTRY_LINK_EXPECTED_PATTERN
                    "multiple definition|duplicate symbol|already defined|LNK2005")
            else()
                set(NEOWPF_ENTRY_LINK_EXPECTED_PATTERN
                    "undefined reference|undefined symbol|unresolved external|LNK2019")
            endif()
            add_executable(${NEOWPF_ENTRY_LINK_TARGET} EXCLUDE_FROM_ALL
                "tests/Automated/${NEOWPF_ENTRY_LINK_FIXTURE}.cpp"
                $<TARGET_OBJECTS:NeoWPF.DesktopEntry>
            )
            add_dependencies(${NEOWPF_ENTRY_LINK_TARGET} NeoWPF.DesktopEntry)
            target_link_libraries(${NEOWPF_ENTRY_LINK_TARGET} PRIVATE NeoWPF::NeoWPF)
            if(WIN32)
                target_link_libraries(${NEOWPF_ENTRY_LINK_TARGET} PRIVATE shell32)
            endif()
            add_test(
                NAME "NeoWPF.${NEOWPF_ENTRY_LINK_FIXTURE}.ExpectedLinkFailure"
                COMMAND "${CMAKE_COMMAND}"
                    "-DBUILD_DIR=${CMAKE_BINARY_DIR}"
                    "-DTARGET=${NEOWPF_ENTRY_LINK_TARGET}"
                    "-DCONFIG=$<CONFIG>"
                    "-DEXPECTED_PATTERN=${NEOWPF_ENTRY_LINK_EXPECTED_PATTERN}"
                    -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/Automated/ExpectEntryLinkFailure.cmake"
            )
        endforeach()
    endif()

    if(NEOWPF_BUILD_DOTNET)
        add_executable(NeoWPF.DotNet.AutomatedTests
            tests/Automated/DotNet/DotNetHostTests.cpp
            tests/Automated/DotNet/DotNetValueTests.cpp
        )
        set(NEOWPF_DOTNET_PLUGIN_DIR
            "${CMAKE_CURRENT_BINARY_DIR}/generated/dotnet/fixture-bindings/$<CONFIG>/plugin")
        target_link_libraries(NeoWPF.DotNet.AutomatedTests
            PRIVATE
                NeoWPF::DotNet
                GTest::gtest_main
        )
        add_library(NeoWPF.DotNet.TestNativeDependency SHARED
            tests/DotNetFixtures/PluginAssembly/NativeDependency.cpp
        )
        set_target_properties(NeoWPF.DotNet.TestNativeDependency PROPERTIES
            OUTPUT_NAME "NeoWPF.DotNet.TestNativeDependency"
        )
        add_custom_command(TARGET NeoWPF.DotNet.TestNativeDependency POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${NEOWPF_DOTNET_PLUGIN_DIR}"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:NeoWPF.DotNet.TestNativeDependency>"
                "${NEOWPF_DOTNET_PLUGIN_DIR}/$<TARGET_FILE_NAME:NeoWPF.DotNet.TestNativeDependency>"
            VERBATIM
        )
        add_dependencies(NeoWPF.DotNet.AutomatedTests NeoWPF.DotNet.TestNativeDependency)
        target_compile_definitions(NeoWPF.DotNet.AutomatedTests PRIVATE
            NEOWPF_INTERNAL_BUILD
            "NEOWPF_DOTNET_BINDING_DIRECTORY=\"${CMAKE_CURRENT_BINARY_DIR}/generated/dotnet/fixture-bindings/$<CONFIG>\""
            "NEOWPF_DOTNET_PLUGIN_DIRECTORY=\"${CMAKE_CURRENT_BINARY_DIR}/generated/dotnet/fixture-bindings/$<CONFIG>/plugin\""
        )
        set(NEOWPF_DOTNET_FIXTURE_BINDING_DIR
            "${CMAKE_CURRENT_BINARY_DIR}/generated/dotnet/fixture-bindings/$<CONFIG>")
        set(NEOWPF_DOTNET_FIXTURE_PROJECT
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/BasicAssembly/BasicAssembly.csproj")
        set(NEOWPF_DOTNET_FIXTURE_SOURCE
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/BasicAssembly/BasicAssembly.cs")
        set(NEOWPF_DOTNET_FIXTURE_ASSEMBLY
            "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/managed/BasicAssembly.dll")
        set(NEOWPF_DOTNET_PLUGIN_ASSEMBLY "${NEOWPF_DOTNET_PLUGIN_DIR}/PluginAssembly.dll")
        set(NEOWPF_DOTNET_GENERATED_BINDING_FILES
            "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/BasicAssembly.g.h"
            "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/BasicAssembly.g.cpp"
            "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/BasicAssembly.Contract.g.cs"
            "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/BasicAssembly.Bindings.csproj"
            "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/BasicAssembly.contract"
            "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/managed/BasicAssembly.Bindings.dll"
        )
        set(NEOWPF_DOTNET_GENERATOR_DLL
            "${CMAKE_CURRENT_SOURCE_DIR}/dotnet/NeoWPF/Interop/Generator/bin/$<CONFIG>/net10.0/NeoWPF.DotNet.Generator.dll")
        add_custom_command(
            OUTPUT "${NEOWPF_DOTNET_FIXTURE_ASSEMBLY}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory
                "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/managed"
            COMMAND "${NEOWPF_DOTNET_EXECUTABLE}" build
                "${NEOWPF_DOTNET_FIXTURE_PROJECT}"
                --configuration $<CONFIG>
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/BasicAssembly/bin/$<CONFIG>/net10.0/BasicAssembly.dll"
                "${NEOWPF_DOTNET_FIXTURE_ASSEMBLY}"
            DEPENDS
                "${NEOWPF_DOTNET_FIXTURE_PROJECT}"
                "${NEOWPF_DOTNET_FIXTURE_SOURCE}"
            VERBATIM
        )
        add_custom_target(NeoWPF.DotNet.BasicAssembly
            DEPENDS "${NEOWPF_DOTNET_FIXTURE_ASSEMBLY}"
        )
        add_custom_command(
            OUTPUT "${NEOWPF_DOTNET_PLUGIN_ASSEMBLY}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${NEOWPF_DOTNET_PLUGIN_DIR}"
            COMMAND "${NEOWPF_DOTNET_EXECUTABLE}" build
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/PluginAssembly/PluginAssembly.csproj"
                --configuration $<CONFIG>
                --output "${NEOWPF_DOTNET_PLUGIN_DIR}"
            DEPENDS
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/PluginAssembly/PluginAssembly.csproj"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/PluginAssembly/PluginService.cs"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/SecondaryDependency/SecondaryDependency.csproj"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/SecondaryDependency/Calculator.cs"
            VERBATIM
        )
        add_custom_target(NeoWPF.DotNet.PluginAssembly
            DEPENDS "${NEOWPF_DOTNET_PLUGIN_ASSEMBLY}"
        )
        add_custom_command(
            OUTPUT ${NEOWPF_DOTNET_GENERATED_BINDING_FILES}
            COMMAND "${NEOWPF_DOTNET_EXECUTABLE}" exec "${NEOWPF_DOTNET_GENERATOR_DLL}"
                generate
                --binding "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/BasicAssembly/AddTen.nwbind"
                --assembly "${NEOWPF_DOTNET_FIXTURE_ASSEMBLY}"
                --output "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}"
            COMMAND "${NEOWPF_DOTNET_EXECUTABLE}" build
                "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/BasicAssembly.Bindings.csproj"
                --configuration $<CONFIG>
                --output "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/managed"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/BasicAssembly/bin/$<CONFIG>/net10.0/BasicAssembly.dll"
                "${NEOWPF_DOTNET_FIXTURE_ASSEMBLY}"
            DEPENDS
                "${NEOWPF_DOTNET_GENERATOR_DLL}"
                "${NEOWPF_DOTNET_FIXTURE_ASSEMBLY}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/DotNetFixtures/BasicAssembly/AddTen.nwbind"
            VERBATIM
        )
        add_custom_target(NeoWPF.DotNet.GeneratedBinding
            DEPENDS ${NEOWPF_DOTNET_GENERATED_BINDING_FILES}
        )
        add_dependencies(NeoWPF.DotNet.GeneratedBinding NeoWPF.DotNet.Generator)
        add_dependencies(NeoWPF.DotNet.GeneratedBinding NeoWPF.DotNet.BasicAssembly)
        add_dependencies(NeoWPF.DotNet.AutomatedTests NeoWPF.DotNet.GeneratedBinding)
        add_dependencies(NeoWPF.DotNet.AutomatedTests NeoWPF.DotNet.PluginAssembly)
        target_sources(NeoWPF.DotNet.AutomatedTests PRIVATE
            "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}/BasicAssembly.g.cpp"
        )
        target_include_directories(NeoWPF.DotNet.AutomatedTests PRIVATE
            "${NEOWPF_DOTNET_FIXTURE_BINDING_DIR}"
        )
        if(MSVC)
            add_custom_command(TARGET NeoWPF.DotNet.AutomatedTests POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${NEOWPF_DOTNET_NETHOST_DIR}/nethost.dll"
                    "$<TARGET_FILE_DIR:NeoWPF.DotNet.AutomatedTests>"
                VERBATIM
            )
        endif()
        add_custom_target(NeoWPF.DotNet.BridgeDeployment
            COMMAND "${CMAKE_COMMAND}" -E make_directory
                "$<TARGET_FILE_DIR:NeoWPF.DotNet.AutomatedTests>/NeoWPF.DotNet"
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${NEOWPF_DOTNET_BRIDGE_OUTPUT_DIR}"
                "$<TARGET_FILE_DIR:NeoWPF.DotNet.AutomatedTests>/NeoWPF.DotNet"
            DEPENDS NeoWPF.DotNet.Bridge
            VERBATIM
        )
        add_dependencies(NeoWPF.DotNet.AutomatedTests NeoWPF.DotNet.BridgeDeployment)
        gtest_discover_tests(NeoWPF.DotNet.AutomatedTests)

        set(NEOWPF_DOTNET_TEST_SCRIPT
            "${CMAKE_CURRENT_BINARY_DIR}/RunNeoWPFDotNetTests.cmake")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RunNeoWPFDotNetTests.cmake"
            "${NEOWPF_DOTNET_TEST_SCRIPT}"
            COPYONLY
        )
        add_test(
            NAME NeoWPF.DotNet.Generator.Tests
            COMMAND "${CMAKE_COMMAND}"
                -DDOTNET_EXECUTABLE=${NEOWPF_DOTNET_EXECUTABLE}
                -DTEST_PROJECT=${NEOWPF_DOTNET_GENERATOR_TEST_PROJECT}
                -P "${NEOWPF_DOTNET_TEST_SCRIPT}"
        )
    endif()
endif()
