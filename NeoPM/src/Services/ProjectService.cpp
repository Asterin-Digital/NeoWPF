#include <NeoPM/Services/ProjectService.h>
#include <NeoPM/Core/Commands/NewCommand.h>

namespace NeoPM::Services {

namespace {

constexpr const char* DefaultProjectType = "vcxproj";
constexpr const char* DefaultSdkSource = "production";
constexpr const char* DefaultSdkConfiguration = "debug";

}

int ProjectService::Create(const ProjectCreationRequest& request)
{
    const auto& projectType = request.type.empty() ? DefaultProjectType : request.type;
    const auto& sdkSource = request.sdkSource.empty() ? DefaultSdkSource : request.sdkSource;
    const auto& sdkConfiguration = request.sdkConfiguration.empty() ? DefaultSdkConfiguration : request.sdkConfiguration;

    Core::Commands::NewCommand command(
        request.name,
        request.outputPath,
        projectType,
        sdkSource,
        sdkConfiguration,
        request.sdkIncludePath,
        request.sdkLibraryPath);
    return command.Execute();
}

}
