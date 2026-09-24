/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Resources/ResourceManifest.h>

#include <filesystem>
#include <string>

namespace NeoPM::Core::Resources {

class XamlCodegen {
public:
    bool Generate(const ResourceManifest& manifest,
                  const std::filesystem::path& projectRoot,
                  const std::string& projectName) const;
};

}
