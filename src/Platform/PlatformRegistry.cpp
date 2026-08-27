#include <NeoWPF/Platform/PlatformRegistry.h>

#include <mutex>
#include <utility>

namespace NeoWPF::Platform {
namespace {

std::mutex& RegistryMutex()
{
    static std::mutex mutex;
    return mutex;
}

std::unique_ptr<NeoWPF::Contracts::IPlatform>& RegisteredPlatform()
{
    static std::unique_ptr<NeoWPF::Contracts::IPlatform> platform;
    return platform;
}

}

void RegisterPlatform(std::unique_ptr<NeoWPF::Contracts::IPlatform> platform)
{
    std::lock_guard<std::mutex> lock(RegistryMutex());
    RegisteredPlatform() = std::move(platform);
}

NeoWPF::Contracts::IPlatform* CurrentPlatform() noexcept
{
    std::lock_guard<std::mutex> lock(RegistryMutex());
    return RegisteredPlatform().get();
}

bool HasPlatform() noexcept
{
    return CurrentPlatform() != nullptr;
}

}
