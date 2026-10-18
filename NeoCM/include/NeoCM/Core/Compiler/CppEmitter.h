/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoCM/Core/Compiler/ResolvedXaml.h>

#include <string>

namespace NeoCM::Core::Compiler {

struct CppEmitterOptions {
    std::string namespaceName;
    std::string componentName;
    std::string headerInclude;
};

struct CppGenerationResult {
    bool success = false;
    std::string header;
    std::string source;
    std::string error;
};

CppGenerationResult EmitCppComponent(const ResolvedXamlDocument& document,
                                     const CppEmitterOptions& options);

}
