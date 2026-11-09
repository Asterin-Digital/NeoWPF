#include <NeoPM/Services/ProjectService.h>
#include <NeoPM/Core/Commands/NewCommand.h>

namespace NeoPM::Services {

namespace {

constexpr const char* DefaultSdkSource = "production";
constexpr const char* DefaultSdkConfiguration = "debug";

}

int ProjectService::Create(const ProjectCreationRequest& request)
{
    const auto& sdkSource = request.sdkSource.empty() ? DefaultSdkSource : request.sdkSource;
    const auto& sdkConfiguration = request.sdkConfiguration.empty() ? DefaultSdkConfiguration : request.sdkConfiguration;

    Core::Commands::NewCommand command(
        request.name,
        request.outputPath,
        sdkSource,
        sdkConfiguration,
        request.sdkIncludeDirectories,
        request.sdkLibraryPath);
    return command.Execute();
}

}
