#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <mutex>
#include <system_error>
#include <unordered_map>

namespace NeoWPF::Core {
namespace {

std::mutex s_storageDirectoriesMutex;
std::unordered_map<std::string, std::filesystem::path> s_storageDirectories;

}

NeoWPF::Result<std::filesystem::path> Application::GetStorageDirectory(const std::string& uuid)
{
    if (!NeoWPF::Platform::IsValidUuid(uuid)) {
        return NeoWPF::Fail<std::string>{"Application storage identifier must be a dashed UUID."};
    }

    {
        std::lock_guard<std::mutex> lock(s_storageDirectoriesMutex);
        const auto iterator = s_storageDirectories.find(uuid);
        if (iterator != s_storageDirectories.end()) {
            return NeoWPF::Success<std::filesystem::path>{iterator->second};
        }
    }

    auto* const platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform) {
        return NeoWPF::Fail<std::string>{"Application storage is unavailable because no platform is registered."};
    }

    auto baseDirectoryResult = platform->GetAppDataDirectory();
    if (baseDirectoryResult.Fail()) {
        return NeoWPF::Fail<std::string>{baseDirectoryResult.Error()};
    }

    const auto& baseDirectory = baseDirectoryResult.Value();
    if (baseDirectory.empty() || !baseDirectory.is_absolute()) {
        return NeoWPF::Fail<std::string>{"The platform returned an invalid application data directory."};
    }

    const auto storageDirectory = baseDirectory / "Asterin Digital" / "Apps" / uuid;
    std::error_code error;
    std::filesystem::create_directories(storageDirectory, error);
    if (error) {
        return NeoWPF::Fail<std::string>{
            "Failed to create application storage directory: " + error.message()};
    }

    error.clear();
    if (!std::filesystem::is_directory(storageDirectory, error) || error) {
        return NeoWPF::Fail<std::string>{
            "Application storage path is not a directory: " + storageDirectory.string()};
    }

    {
        std::lock_guard<std::mutex> lock(s_storageDirectoriesMutex);
        const auto [iterator, inserted] = s_storageDirectories.emplace(uuid, storageDirectory);
        if (!inserted) {
            return NeoWPF::Success<std::filesystem::path>{iterator->second};
        }
    }

    return NeoWPF::Success<std::filesystem::path>{storageDirectory};
}

}