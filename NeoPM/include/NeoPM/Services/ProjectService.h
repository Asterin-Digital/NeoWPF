/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace NeoPM::Services {

struct ProjectCreationRequest {
    std::string name;
    std::optional<std::string> outputPath;
    std::string sdkSource;
    std::string sdkConfiguration;
    std::vector<std::string> sdkIncludeDirectories;
    std::optional<std::string> sdkLibraryPath;
};

class ProjectService final {
public:
    static int Create(const ProjectCreationRequest& request);
};

}
