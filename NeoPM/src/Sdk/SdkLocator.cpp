#include <NeoPM/Sdk/SdkLocator.h>
#include <NeoPM/Utils/Logger.h>

#include <utility>

namespace NeoPM::Core {

SdkLocator::SdkLocator(const std::filesystem::path& sdkRoot, std::string platform)
    : m_sdkRoot(sdkRoot), m_platform(std::move(platform))
{
}

std::optional<std::filesystem::path> SdkLocator::FindSdk(
    const std::string& architecture) const
{
    auto sdkFolder = m_sdkRoot / (m_platform + "-" + architecture);
    if (std::filesystem::is_directory(sdkFolder)) {
        return sdkFolder;
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> SdkLocator::FindLibrary(
    const std::string& configuration,
    const std::string& architecture) const
{
    auto sdkPath = FindSdk(architecture);
    if (!sdkPath) {
        return std::nullopt;
    }

    try {
        const auto configurationName = configuration == "debug" ? "Debug" :
            configuration == "release" ? "Release" : "";
        if (configurationName.empty()) {
            return std::nullopt;
        }

        const auto configurationPath = sdkPath.value() / configurationName;
        for (const auto& entry : std::filesystem::directory_iterator(configurationPath)) {
            if (entry.is_regular_file() &&
                (entry.path().extension() == ".lib" || entry.path().extension() == ".a")) {
                return entry.path();
            }
        }
    }
    catch (const std::exception& e) {
        auto& logger = Utils::Logger::Instance();
        logger.Debug("SdkLocator", "Error searching platform configuration library: " + std::string(e.what()));
    }

    return std::nullopt;
}

std::optional<std::filesystem::path> SdkLocator::FindIncludeDirectory(
    const std::string& architecture) const
{
    auto sdkPath = FindSdk(architecture);
    if (!sdkPath) {
        return std::nullopt;
    }

    try {
        const auto includePath = sdkPath.value() / "include";
        if (std::filesystem::is_directory(includePath / "NeoWPF") &&
            std::filesystem::is_directory(includePath / "NeoNaml")) {
            return includePath;
        }
    }
    catch (const std::exception& e) {
        auto& logger = Utils::Logger::Instance();
        logger.Debug("SdkLocator", "Error searching for include directory: " + std::string(e.what()));
    }

    return std::nullopt;
}

bool SdkLocator::HasAnySdk() const {
    if (!std::filesystem::exists(m_sdkRoot)) {
        return false;
    }

    try {
        for (const auto& entry : std::filesystem::directory_iterator(m_sdkRoot)) {
            if (!entry.is_directory() ||
                entry.path().filename().string().rfind(m_platform + "-", 0) != 0) {
                continue;
            }

            auto architecture = entry.path().filename().string().substr(m_platform.size() + 1);
            if (FindIncludeDirectory(architecture) &&
                FindLibrary("debug", architecture) &&
                FindLibrary("release", architecture)) {
                return true;
            }
        }
    }
    catch (const std::exception& e) {
        auto& logger = Utils::Logger::Instance();
        logger.Debug("SdkLocator", "Error checking for local SDKs: " + std::string(e.what()));
    }

    return false;
}

}
