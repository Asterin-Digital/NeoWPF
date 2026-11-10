#include <NeoWPF/Platform/Android/AWindow.h>
#include <NeoWPF/Platform/Android/APlatform.h>

#include <NeoWPF/Core/Application/ApplicationInternal.h>
#include <NeoWPF/Core/Application/Resources/Embedded/FallbackFont.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformUtils.h>
#include <NeoWPF/Rendering/Text/FontDatabase.h>
#include <NeoWPF/Rendering/TextRenderer.h>

#include <dlfcn.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <chrono>
#include <mutex>

namespace NeoWPF::Platform::Android {
namespace {

class AClipboard final : public NeoWPF::Contracts::IClipboard {
public:
    bool SetText(const std::wstring&) override { return false; }
    void TryGetText(TextCallback callback) override
    {
        if (callback) {
            callback({});
        }
    }
    bool Clear() override { return false; }
};

bool LoadEmbeddedFallbackFont()
{
    using NeoWPF::Core::ApplicationResources::Embedded::FontRecord;
    using NeoWPF::Core::ApplicationResources::Embedded::GetEmbeddedFallbackFontCount;
    using NeoWPF::Core::ApplicationResources::Embedded::TryGetEmbeddedFallbackFont;

    for (std::size_t index = 0; index < GetEmbeddedFallbackFontCount(); ++index) {
        FontRecord record{};
        if (!TryGetEmbeddedFallbackFont(index, record) || !record.data || record.size == 0) {
            continue;
        }

        const auto familyName = NeoWPF::Platform::Utf8ToWide(record.name ? record.name : "Noto Sans");
        if (NeoWPF::Rendering::Text::RegisterFontMemory(record.data, record.size, familyName)) {
            return true;
        }
    }
    return false;
}

}

APlatform::APlatform(std::filesystem::path applicationDataDirectory)
        : m_clipboard(std::make_unique<AClipboard>()),
            m_applicationDataDirectory(std::move(applicationDataDirectory))
{
}

APlatform::~APlatform() = default;

NeoWPF::Contracts::IClipboard& APlatform::Clipboard() noexcept { return *m_clipboard; }
NeoWPF::Contracts::IWindowing& APlatform::Windowing() noexcept { return m_windowing; }
NeoWPF::Contracts::IRendering& APlatform::Rendering() noexcept { return m_rendering; }
NeoWPF::Contracts::IWindowChrome* APlatform::Chrome() noexcept { return nullptr; }
NeoWPF::Platform::Capabilities::PlatformId APlatform::GetPlatformId() const noexcept
{
    return NeoWPF::Platform::Capabilities::PlatformId::Android;
}

NeoWPF::Core::DeviceCategory APlatform::GetDeviceCategory() const noexcept
{
    return NeoWPF::Core::DeviceCategory::Mobile;
}

int APlatform::RunApplication(std::chrono::seconds startupTimeout)
{
    if (!m_windowing.WaitForSurface(startupTimeout)) {
        m_windowing.ShowFatalError(L"NeoWPF Error", L"Timed out waiting for the Android surface.");
        m_windowing.Stop();
        return -1;
    }
    const int result = NeoWPF::Core::RunDesktopApp(startupTimeout);
    m_windowing.Stop();
    return result;
}

NeoWPF::Result<std::vector<std::uint8_t>> APlatform::LoadResourceData(const std::wstring&)
{
    return NeoWPF::Fail<std::string>{"Android embedded resource loading is not implemented"};
}

std::filesystem::path APlatform::GetExecutableDirectory() const
{
    return NeoWPF::Fail<std::string>{"Android document picker is not implemented"};
}

NeoWPF::Result<std::filesystem::path> APlatform::GetAppDataDirectory() const
{
    if (m_applicationDataDirectory.empty() || !m_applicationDataDirectory.is_absolute()) {
        return NeoWPF::Fail<std::string>{"Android did not provide a valid private application data directory."};
    }

    return NeoWPF::Success<std::filesystem::path>{m_applicationDataDirectory};
}

NeoWPF::Result<std::vector<std::uint8_t>> APlatform::FetchHttpResource(const std::wstring&)
{
    return NeoWPF::Fail<std::string>{"Android HTTP resource fetching is not implemented"};
}

NeoWPF::Contracts::FileDialogResult APlatform::ShowOpenFileDialog(
    const std::wstring&, const std::wstring&,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>&, bool)
{
    return NeoWPF::Fail<std::string>{"Android document picker is not implemented"};
}

NeoWPF::Contracts::FileDialogResult APlatform::ShowSaveFileDialog(
    const std::wstring&, const std::wstring&, const std::wstring&,
    const std::vector<NeoWPF::Contracts::FileDialogFilter>&)
{
    return {};
}

NeoWPF::Result<NeoWPF::Contracts::DecodedImage> APlatform::DecodeImageFromMemory(
    const std::vector<std::uint8_t>&, int, int)
{
    return NeoWPF::Fail<std::string>{"Android image decoding is not implemented"};
}

NeoWPF::Result<NeoWPF::Contracts::DecodedImage> APlatform::DecodeImageFile(
    const std::wstring&, int, int)
{
    return NeoWPF::Fail<std::string>{"Android image decoding is not implemented"};
}

NeoWPF::Result<void> APlatform::RegisterFontFile(const std::wstring& filePath)
{
    if (!NeoWPF::Rendering::Text::RegisterFontFile(filePath)) {
        return NeoWPF::Fail<std::string>{"Font file could not be registered"};
    }
    return NeoWPF::Success<void>{};
}

NeoWPF::Result<void> APlatform::RegisterFontMemory(
    const unsigned char* data, std::size_t size, const std::wstring& familyName)
{
    if (!NeoWPF::Rendering::Text::RegisterFontMemory(data, size, familyName)) {
        return NeoWPF::Fail<std::string>{"Font data could not be registered"};
    }
    return NeoWPF::Success<void>{};
}

std::wstring APlatform::ResolveFontFamily(const std::wstring& requestedFontFamily)
{
    auto& fontDatabase = NeoWPF::Rendering::Text::GetFontDatabase();
    if (fontDatabase.Records().empty()) {
        static std::once_flag fallbackFontRegistration;
        std::call_once(fallbackFontRegistration, [] {
            if (NeoWPF::Rendering::Text::GetFontDatabase().Records().empty() &&
                !LoadEmbeddedFallbackFont()) {
                NeoWPF::Diagnostics::LogWarn(
                    "Android embedded fallback font could not be registered");
            }
        });
    }
    return NeoWPF::Rendering::Text::ResolveFontFamily(requestedFontFamily);
}

int APlatform::CaptureCallStack(NeoWPF::Diagnostics::CallStackEntry*, int, int) noexcept
{
    return 0;
}

void APlatform::InstallCrashHandler() noexcept
{
    NeoWPF::Diagnostics::LogInfo("Android crash handler is not installed yet");
}

void APlatform::UninstallCrashHandler() noexcept
{
}

std::wstring APlatform::CurrentModulePath() const
{
    Dl_info moduleInfo{};
    const auto symbol = dlsym(RTLD_DEFAULT, "ANativeActivity_onCreate");
    if (!symbol || dladdr(symbol, &moduleInfo) == 0 || !moduleInfo.dli_fname) {
        return {};
    }
    return NeoWPF::Platform::Utf8ToWide(moduleInfo.dli_fname);
}

std::uint32_t APlatform::CurrentThreadId() const noexcept
{
    return static_cast<std::uint32_t>(::syscall(SYS_gettid));
}

}