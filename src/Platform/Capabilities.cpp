#include <NeoWPF/Platform/Capabilities.h>
#include <NeoWPF/Contracts/IPlatform.h>
#include <NeoWPF/Platform/PlatformRegistry.h>

namespace NeoWPF::Platform::Capabilities
{
    CapabilityStatus GetStatus(
        PlatformId platform,
        CapabilityGroupId group,
        CapabilityId capability) noexcept
    {
        switch (platform)
        {
        case PlatformId::Android:
        case PlatformId::Linux:
        case PlatformId::Windows:
            break;
        default:
            return CapabilityStatus::Undeclared;
        }

        switch (group)
        {
        case CapabilityGroupId::Windowing:
            if (capability == CapabilityId::MultiWindow)
                return platform == PlatformId::Android
                    ? CapabilityStatus::Unsupported
                    : CapabilityStatus::Supported;
            break;
        case CapabilityGroupId::WindowChrome:
            if (capability == CapabilityId::ChromeService ||
                capability == CapabilityId::ChromeNativeMode ||
                capability == CapabilityId::ChromeCustomMode)
            {
                return platform == PlatformId::Android
                    ? CapabilityStatus::Unsupported
                    : CapabilityStatus::Supported;
            }
            if (capability == CapabilityId::ChromeNoneMode)
                return platform == PlatformId::Linux
                    ? CapabilityStatus::Supported
                    : CapabilityStatus::Unsupported;
            if (capability == CapabilityId::NativeCaptionColor)
                return platform == PlatformId::Windows
                    ? CapabilityStatus::Supported
                    : CapabilityStatus::Unsupported;
            break;
        case CapabilityGroupId::FileDialogs:
            if (capability == CapabilityId::OpenFileDialog ||
                capability == CapabilityId::SaveFileDialog ||
                capability == CapabilityId::MultiSelectFileDialog)
            {
                return CapabilityStatus::Supported;
            }
            break;
        case CapabilityGroupId::Clipboard:
            if (capability == CapabilityId::TextClipboard)
                return CapabilityStatus::Supported;
            break;
        case CapabilityGroupId::Resources:
            if (capability == CapabilityId::EmbeddedResourceLoading)
                return CapabilityStatus::Supported;
            break;
        case CapabilityGroupId::Networking:
            if (capability == CapabilityId::HttpResourceFetch)
                return CapabilityStatus::Supported;
            break;
        case CapabilityGroupId::Images:
            if (capability == CapabilityId::ImageDecodeFromMemory ||
                capability == CapabilityId::ImageDecodeFromFile)
            {
                return CapabilityStatus::Supported;
            }
            break;
        }

        return CapabilityStatus::Undeclared;
    }

    CapabilityStatus GetStatus(
        NeoWPF::Core::DeviceCategory category,
        CapabilityGroupId group,
        CapabilityId capability) noexcept
    {
        if (category == NeoWPF::Core::DeviceCategory::Mobile)
            return GetStatus(PlatformId::Android, group, capability);

        if (category != NeoWPF::Core::DeviceCategory::Desktop)
            return CapabilityStatus::Undeclared;

        const CapabilityStatus linuxStatus = GetStatus(PlatformId::Linux, group, capability);
        const CapabilityStatus windowsStatus = GetStatus(PlatformId::Windows, group, capability);
        if (linuxStatus == CapabilityStatus::Undeclared ||
            windowsStatus == CapabilityStatus::Undeclared)
        {
            return CapabilityStatus::Undeclared;
        }
        if (linuxStatus == CapabilityStatus::Unsupported ||
            windowsStatus == CapabilityStatus::Unsupported)
        {
            return CapabilityStatus::Unsupported;
        }
        return CapabilityStatus::Supported;
    }

    bool IsSupported(
        PlatformId platform,
        CapabilityGroupId group,
        CapabilityId capability) noexcept
    {
        return GetStatus(platform, group, capability) == CapabilityStatus::Supported;
    }

    bool IsSupported(
        NeoWPF::Core::DeviceCategory category,
        CapabilityGroupId group,
        CapabilityId capability) noexcept
    {
        return GetStatus(category, group, capability) == CapabilityStatus::Supported;
    }

    CapabilityStatus GetCurrentStatus(
        CapabilityGroupId group,
        CapabilityId capability) noexcept
    {
        const auto* platform = NeoWPF::Platform::CurrentPlatform();
        return platform != nullptr
            ? GetStatus(platform->GetPlatformId(), group, capability)
            : CapabilityStatus::Undeclared;
    }

    bool IsCurrentSupported(
        CapabilityGroupId group,
        CapabilityId capability) noexcept
    {
        return GetCurrentStatus(group, capability) == CapabilityStatus::Supported;
    }
}