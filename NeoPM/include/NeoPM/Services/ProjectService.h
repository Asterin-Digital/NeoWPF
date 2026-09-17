#pragma once

#include <optional>
#include <string>

namespace NeoPM::Services {

struct ProjectCreationRequest {
    std::string name;
    std::optional<std::string> outputPath;
    std::string type;
    std::string sdkSource;
    std::string sdkConfiguration;
    std::optional<std::string> sdkIncludePath;
    std::optional<std::string> sdkLibraryPath;
};

class ProjectService final {
public:
    static int Create(const ProjectCreationRequest& request);
};

}
