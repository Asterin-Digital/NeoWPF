/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoNaml/NamlArtifact.h>

#include <string>

namespace NeoCM::Core::Compiler {

struct NamlCompileResult {
    bool success = false;
    NeoNaml::NamlArtifact artifact;
    std::string error;
};

NamlCompileResult CompileXaml(const std::string& source, const std::string& sourcePath);

}
