#include <NeoWPF/Core/Application/Resources/ResourceManager.h>

#include <NeoWPF/Core/Application/Resources/EmbeddedResourceProvider.h>
#include <NeoWPF/Core/Application/Resources/ExternalFileResourceProvider.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Platform/PlatformRegistry.h>

#include <filesystem>
#include <fstream>

namespace NeoWPF {

ResourceManager& ResourceManager::Instance()
{
    static ResourceManager manager;
    return manager;
}

void ResourceManager::RegisterProvider(std::unique_ptr<IResourceProvider> provider)
{
    if (provider) {
        std::lock_guard lock(m_mutex);
        m_providers.emplace_back(std::move(provider));
    }
}

void ResourceManager::RegisterProviderReference(IResourceProvider& provider)
{
    std::lock_guard lock(m_mutex);
    m_providerReferences.emplace_back(&provider);
}

bool ResourceManager::TryGetBlob(const ResourceKey& key, ResourceBlob& outBlob)
{
    std::vector<std::shared_ptr<IResourceProvider>> providers;
    std::vector<IResourceProvider*> providerReferences;
    {
        std::lock_guard lock(m_mutex);
        providers = m_providers;
        providerReferences = m_providerReferences;
    }

    for (const auto& provider : providers) {
        if (provider && provider->TryGet(key, outBlob)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Debug,
                "[ResourceManager] Resolved resource '%s' via provider %p (%zu bytes)",
                key.Name(), provider.get(), outBlob.Size);
            return true;
        }
    }

    for (const auto& provider : providerReferences) {
        if (provider && provider->TryGet(key, outBlob)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Debug,
                "[ResourceManager] Resolved resource '%s' via provider %p (%zu bytes)",
                key.Name(), provider, outBlob.Size);
            return true;
        }
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
        "[ResourceManager] No provider could resolve resource '%s'",
        key.Name());
    return false;
}

std::shared_ptr<UI::Core::Media::BitmapSource> ResourceManager::FindImage(const ResourceKey& key)
{
    ResourceBlob blob{};
    if (!TryGetBlob(key, blob) || !blob.Data || blob.Size == 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
            "[ResourceManager] FindImage failed for '%s' (blob missing or empty)", key.Name());
        return nullptr;
    }

    std::vector<uint8_t> pixels;
    int width = 0;
    int height = 0;
    auto* platform = NeoWPF::Platform::CurrentPlatform();
    if (!platform || !platform->DecodeImageFromMemory(std::vector<uint8_t>(blob.Data, blob.Data + blob.Size), pixels, width, height, 0, 0)) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
            "[ResourceManager] DecodeImageFromMemory failed for '%s' (%zu bytes)", key.Name(), blob.Size);
        return nullptr;
    }

    return UI::Core::Media::BitmapSource::Create(width, height, std::move(pixels));
}

std::shared_ptr<IDWriteFontFace> ResourceManager::FindFont(const ResourceKey& key)
{
    ResourceBlob blob{};
    if (!TryGetBlob(key, blob) || !blob.Data || blob.Size == 0) {
        return nullptr;
    }

    (void)key;
    return nullptr;
}

ResourceBlob ResourceManager::FindRaw(const ResourceKey& key)
{
    ResourceBlob blob{};
    TryGetBlob(key, blob);
    return blob;
}

}
