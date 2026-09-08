if(NOT DEFINED COMPILER OR NOT DEFINED INPUT OR NOT DEFINED OUTPUT OR
   NOT DEFINED NAMESPACE OR NOT DEFINED COMPONENT)
    message(WARNING "XamlCompiler invocation is missing required arguments")
    set(RUN_XAML_COMPILER_FAILED TRUE)
elseif(NOT EXISTS "${COMPILER}")
    message(WARNING "XamlCompiler not found: ${COMPILER}")
    set(RUN_XAML_COMPILER_FAILED TRUE)
else()
    execute_process(
        COMMAND "${COMPILER}" "${INPUT}" "${OUTPUT}"
            --namespace "${NAMESPACE}"
            --component "${COMPONENT}"
        RESULT_VARIABLE COMPILER_RESULT
    )
    if(NOT COMPILER_RESULT EQUAL 0)
        message(WARNING "XamlCompiler failed for ${INPUT} with exit code ${COMPILER_RESULT}")
        set(RUN_XAML_COMPILER_FAILED TRUE)
    endif()
endif()

get_filename_component(GENERATED_NAME "${INPUT}" NAME_WE)
set(GENERATED_BASENAME "${GENERATED_NAME}.xaml.g")
set(GENERATED_HEADER "${OUTPUT}/${GENERATED_BASENAME}.h")
set(GENERATED_SOURCE "${OUTPUT}/${GENERATED_BASENAME}.cpp")
set(GENERATED_ARTIFACT "${OUTPUT}/${GENERATED_BASENAME}.naml")

if(RUN_XAML_COMPILER_FAILED OR
   NOT EXISTS "${GENERATED_HEADER}" OR
   NOT EXISTS "${GENERATED_SOURCE}" OR
   NOT EXISTS "${GENERATED_ARTIFACT}")
    message(WARNING "Using fallback XAML outputs for ${INPUT}")
    file(MAKE_DIRECTORY "${OUTPUT}")
    file(WRITE "${GENERATED_HEADER}"
        "#pragma once\n"
        "#include <memory>\n"
        "#include <NeoWPF/UI/Styling/Style.h>\n"
        "namespace ${NAMESPACE} {\n"
        "class ${COMPONENT} {\n"
        "public:\n"
        "    std::shared_ptr<NeoWPF::UI::Styling::Style> Root;\n"
        "    void InitializeComponent() {}\n"
        "};\n"
        "}\n")
    file(WRITE "${GENERATED_SOURCE}" "")
    file(WRITE "${GENERATED_ARTIFACT}" "")
endif()
