#include <NeoPM/Sdk/SdkManifest.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>

using json = nlohmann::json;

namespace NeoPM::Core {
namespace {

std::string GetManifestPlatform(const std::string& platform)
{
    if (platform == "Windows") return "windows";
    if (platform == "Linux") return "linux";
    if (platform == "Android") return "android";
    return {};
}

std::string ReadString(const json& object, const char* key)
{
    if (!object.contains(key) || !object[key].is_string()) {
        throw std::invalid_argument(std::string("SDK manifest field must be a string: ") + key);
    }
    return object[key].get<std::string>();
}

std::string ReadConfigurationLibrary(const json& platform, const char* configuration)
{
    if (!platform.contains(configuration) || !platform[configuration].is_object()) {
        throw std::invalid_argument(std::string("SDK manifest configuration is missing: ") + configuration);
    }

    const auto& libraries = platform[configuration]["libraries"];
    if (!libraries.is_array() || libraries.size() != 1 || !libraries.front().is_string()) {
        throw std::invalid_argument(std::string("SDK manifest must provide one library for ") + configuration);
    }

    auto path = libraries.front().get<std::string>();
    if (!SdkManifestParser::IsSafeRelativePath(path)) {
        throw std::invalid_argument(std::string("SDK manifest contains an unsafe library path for ") + configuration);
    }
    return path;
}

}

bool SdkManifestParser::IsSafeRelativePath(const std::string& value)
{
    if (value.empty() || value.find('\\') != std::string::npos ||
        (value.size() > 1 && std::isalpha(static_cast<unsigned char>(value[0])) && value[1] == ':')) {
        return false;
    }

    const std::filesystem::path path(value);
    return !path.is_absolute() && !path.has_root_name() &&
        std::none_of(path.begin(), path.end(), [](const auto& part) {
            return part == "..";
        });
}

std::optional<SdkRelease> SdkManifestParser::Parse(
    const std::string& manifestJson,
    const std::string& platform,
    const std::string& architecture)
{
    const auto manifest = json::parse(manifestJson);
    if (!manifest.is_object() || !manifest.contains("schema_version") ||
        !manifest["schema_version"].is_number_integer() || manifest["schema_version"] != 1) {
        throw std::invalid_argument("SDK manifest has an unsupported schema version");
    }

    SdkRelease release;
    release.version = ReadString(manifest, "version");
    release.downloadUrl = ReadString(manifest, "download_url");
    if (release.version.empty() || release.downloadUrl.empty()) {
        throw std::invalid_argument("SDK manifest version and download_url must not be empty");
    }
    const std::filesystem::path versionPath(release.version);
    if (!IsSafeRelativePath(release.version) || versionPath.has_parent_path() ||
        versionPath.filename().string() != release.version) {
        throw std::invalid_argument("SDK manifest version must be a safe directory name");
    }

    if (!manifest.contains("shared") || !manifest["shared"].is_object()) {
        throw std::invalid_argument("SDK manifest must contain a shared object");
    }
    const auto& shared = manifest["shared"];
    release.licensePath = ReadString(shared, "license");
    if (!IsSafeRelativePath(release.licensePath) ||
        !shared.contains("include_dirs") || !shared["include_dirs"].is_array() ||
        shared["include_dirs"].empty()) {
        throw std::invalid_argument("SDK manifest contains invalid shared paths");
    }

    for (const auto& include : shared["include_dirs"]) {
        if (!include.is_string()) {
            throw std::invalid_argument("SDK manifest include_dirs entries must be strings");
        }
        auto path = include.get<std::string>();
        if (!IsSafeRelativePath(path)) {
            throw std::invalid_argument("SDK manifest contains an unsafe include path");
        }
        release.includePaths.push_back(std::move(path));
    }

    const auto manifestPlatform = GetManifestPlatform(platform);
    if (manifestPlatform.empty()) {
        throw std::invalid_argument("SDK manifest does not support the current platform");
    }
    if (!manifest.contains("platforms") || !manifest["platforms"].is_object()) {
        throw std::invalid_argument("SDK manifest must contain a platforms object");
    }
    const auto& platforms = manifest["platforms"];
    if (!platforms.contains(manifestPlatform)) {
        return std::nullopt;
    }
    const auto& selectedPlatform = platforms[manifestPlatform];
    if (!selectedPlatform.is_object()) {
        throw std::invalid_argument("SDK manifest platform entry must be an object");
    }

    release.debugLibraryPath = ReadConfigurationLibrary(selectedPlatform, "debug");
    release.releaseLibraryPath = ReadConfigurationLibrary(selectedPlatform, "release");
    release.platform = platform;
    release.architecture = architecture;
    return release;
}

}