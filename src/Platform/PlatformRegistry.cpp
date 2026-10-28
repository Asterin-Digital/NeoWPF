#include <NeoWPF/Platform/PlatformRegistry.h>
#include <NeoWPF/Diagnostics/Logger.h>

#ifdef _WIN32
#include <NeoWPF/Platform/Windows/WPlatform.h>
#elif defined(__ANDROID__)
#error Unfinished
#elif defined(__linux__)
#include <NeoWPF/Platform/Linux/LPlatform.h>
#else
#error Unfinished
#endif

#include <mutex>
#include <utility>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

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

std::unique_ptr<NeoWPF::Contracts::IPlatform> CreateDefaultPlatform()
{
#ifdef _WIN32
    return std::make_unique<NeoWPF::Platform::Windows::WPlatform>();
#elif defined(__ANDROID__)
#error Unfinished
#elif defined(__linux__)
    return std::make_unique<NeoWPF::Platform::Linux::LPlatform>();
#else
#error Unfinished
#endif
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

[[noreturn]] void ExitForMissingPlatform()
{
    constexpr auto message = L"NeoWPF could not initialize because no platform backend is available.\n\n"
                             L"Make sure Application::Initialize() is called before using the application.";
    NeoWPF::Diagnostics::LogError(
        "Application initialization failed: no platform backend is available");
#ifdef _WIN32
    ::MessageBoxW(nullptr, message, L"NeoWPF Initialization Error", MB_OK | MB_ICONERROR);
#endif
    std::exit(EXIT_FAILURE);
}

[[noreturn]] void ExitForUninitializedApplication()
{
    constexpr auto message = L"The NeoWPF application was used before Application::Initialize() was called.\n\n"
                             L"Call NeoWPF::Core::Application::Initialize() before accessing the application.";
    NeoWPF::Diagnostics::LogError(
        "Application was used before Application::Initialize() was called");
#ifdef _WIN32
    ::MessageBoxW(nullptr, message, L"NeoWPF Initialization Error", MB_OK | MB_ICONERROR);
#endif
    std::exit(EXIT_FAILURE);
}

}
