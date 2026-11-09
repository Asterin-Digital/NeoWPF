#include <NeoPM/Sdk/SdkArchiveInstaller.h>
#include <NeoPM/Sdk/SdkManifest.h>

#include <algorithm>
#include <fstream>

namespace NeoPM::Core {
namespace {

bool CopyLibrary(
    const std::filesystem::path& archiveRoot,
    const std::filesystem::path& sdkRoot,
    const std::string& relativePath,
    const char* configuration)
{
    if (!SdkManifestParser::IsSafeRelativePath(relativePath)) return false;
    const auto source = archiveRoot / relativePath;
    if (!std::filesystem::is_regular_file(source)) return false;

    std::error_code error;
    const auto destination = sdkRoot / configuration;
    std::filesystem::create_directories(destination, error);
    if (error) return false;
    std::filesystem::copy_file(source, destination / source.filename(),
        std::filesystem::copy_options::overwrite_existing, error);
    return !error;
}

}

bool SdkArchiveInstaller::Install(
    const SdkRelease& release,
    const std::filesystem::path& archiveRoot,
    const std::filesystem::path& sdkRoot)
{
    if (release.includePaths.empty() || !SdkManifestParser::IsSafeRelativePath(release.licensePath) ||
        !CopyLibrary(archiveRoot, sdkRoot, release.debugLibraryPath, "Debug") ||
        !CopyLibrary(archiveRoot, sdkRoot, release.releaseLibraryPath, "Release")) {
        return false;
    }

    const auto licenseSource = archiveRoot / release.licensePath;
    if (!std::filesystem::is_regular_file(licenseSource)) return false;

    std::error_code error;
    std::filesystem::copy_file(licenseSource, sdkRoot / licenseSource.filename(),
        std::filesystem::copy_options::overwrite_existing, error);
    if (error) return false;

    for (const auto& relativePath : release.includePaths) {
        if (!SdkManifestParser::IsSafeRelativePath(relativePath)) return false;
        const auto source = archiveRoot / relativePath;
        if (!std::filesystem::is_directory(source)) return false;

        const auto destination = sdkRoot / relativePath;
        std::filesystem::create_directories(destination.parent_path(), error);
        if (error) return false;
        std::filesystem::copy(source, destination,
            std::filesystem::copy_options::recursive |
                std::filesystem::copy_options::overwrite_existing, error);
        if (error) return false;
    }

    const auto containsHeaderDirectory = [&release, &sdkRoot](const char* name) {
        return std::any_of(release.includePaths.begin(), release.includePaths.end(),
            [&sdkRoot, name](const auto& relativePath) {
                return std::filesystem::is_directory(sdkRoot / relativePath / name);
            });
    };
    if (!containsHeaderDirectory("NeoWPF") || !containsHeaderDirectory("NeoNaml")) {
        return false;
    }

    std::ofstream includePathsFile(sdkRoot / ".include_paths", std::ios::out | std::ios::trunc);
    if (!includePathsFile) return false;
    for (const auto& relativePath : release.includePaths) {
        includePathsFile << relativePath << '\n';
    }
    return static_cast<bool>(includePathsFile);
}

}