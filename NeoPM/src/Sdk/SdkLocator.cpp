#include <NeoPM/Sdk/SdkLocator.h>
#include <NeoPM/Utils/Logger.h>

namespace NeoPM::Core {

SdkLocator::SdkLocator(const std::filesystem::path& sdkRoot)
    : m_sdkRoot(sdkRoot)
{
}

std::optional<std::filesystem::path> SdkLocator::FindSdk(
    const std::string& channel, 
    const std::string& architecture) const
{
    auto sdkFolder = m_sdkRoot / (channel + "-" + architecture);
    if (std::filesystem::exists(sdkFolder)) {
        return sdkFolder;
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> SdkLocator::FindLibrary(
    const std::string& channel, 
    const std::string& architecture) const
{
    auto sdkPath = FindSdk(channel, architecture);
    if (!sdkPath) {
        return std::nullopt;
    }

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(sdkPath.value())) {
            if (entry.is_regular_file() && entry.path().filename() == "NeoWPF.lib") {
                return entry.path();
            }
        }
    }
    catch (const std::exception& e) {
        auto& logger = Utils::Logger::Instance();
        logger.Debug("SdkLocator", "Error searching for NeoWPF.lib: " + std::string(e.what()));
    }

    return std::nullopt;
}

std::optional<std::filesystem::path> SdkLocator::FindIncludeDirectory(
    const std::string& channel, 
    const std::string& architecture) const
{
    auto sdkPath = FindSdk(channel, architecture);
    if (!sdkPath) {
        return std::nullopt;
    }

    try {
        for (const auto& entry : std::filesystem::directory_iterator(sdkPath.value())) {
            if (entry.is_directory() && entry.path().filename() == "include") {
                return entry.path();
            }
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
            if (entry.is_directory()) {
                for (const auto& subEntry : std::filesystem::directory_iterator(entry.path())) {
                    if (subEntry.is_directory() &&
                        (subEntry.path().filename() == "include" || subEntry.path().filename() == "lib")) {
                        return true;
                    }
                }
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
