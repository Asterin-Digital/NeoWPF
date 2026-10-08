#include <NeoWPF/DotNet/Hosting/HostFxr/ManagedBridgeLocation.h>

#include <array>
#include <filesystem>
#include <system_error>
#include <string_view>

namespace NeoWPF::DotNet::Hosting::HostFxr {

namespace {

bool HasBridgeFiles(const std::filesystem::path& directory)
{
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error))
        return false;

    constexpr std::array<std::string_view, 3> bridgeFiles{
        "NeoWPF.DotNet.Bridge.dll",
        "NeoWPF.DotNet.Bridge.runtimeconfig.json",
        "NeoWPF.DotNet.Bridge.deps.json"
    };

    for (const auto file : bridgeFiles)
    {
        error.clear();
        if (!std::filesystem::is_regular_file(directory / file, error))
            return false;
    }

    return true;
}

}

std::filesystem::path ResolveLocation(
    const std::filesystem::path& executableDirectory,
    const std::filesystem::path& configuredDirectory)
{
    if (!configuredDirectory.empty() && HasBridgeFiles(configuredDirectory))
        return configuredDirectory;

    const auto deployedDirectory = executableDirectory / "NeoWPF.DotNet";
    if (HasBridgeFiles(deployedDirectory))
        return deployedDirectory;

    return HasBridgeFiles(executableDirectory) ? executableDirectory : std::filesystem::path{};
}

}
