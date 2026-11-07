#include <NeoPM/Sdk/SdkVersionManager.h>
#include <fstream>

namespace NeoPM::Core {

SdkVersionManager::SdkVersionManager(const std::filesystem::path& sdkRoot)
    : m_sdkRoot(sdkRoot)
{
}

std::filesystem::path SdkVersionManager::GetVersionFilePath(
    const std::string& platform,
    const std::string& architecture) const
{
    return m_sdkRoot / (platform + "-" + architecture) / ".version";
}

std::string SdkVersionManager::GetInstalledVersion(
    const std::string& platform,
    const std::string& architecture) const
{
    auto versionFile = GetVersionFilePath(platform, architecture);

    if (!std::filesystem::exists(versionFile)) {
        return "";
    }

    try {
        std::ifstream file(versionFile);
        if (!file) {
            return "";
        }
        std::string version;
        std::getline(file, version);
        return version;
    }
    catch (...) {
        return "";
    }
}

void SdkVersionManager::SaveInstalledVersion(
    const std::string& platform,
    const std::string& architecture, 
    const std::string& version)
{
    auto versionFile = GetVersionFilePath(platform, architecture);

    try {
        std::ofstream file(versionFile);
        if (file) {
            file << version;
            file.close();
        }
    }
    catch (...) {
    }
}

}
