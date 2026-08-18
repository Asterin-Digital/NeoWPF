#include <NeoWPF/UI/Data/ResourceManager.h>

#include <NeoWPF/UI/Data/EmbeddedResourceProvider.h>
#include <NeoWPF/UI/Data/ExternalFileResourceProvider.h>
#include <NeoWPF/Diagnostics/Logger.h>

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
        m_providers.emplace_back(std::move(provider));
    }
}

void ResourceManager::RegisterProviderReference(IResourceProvider& provider)
{
    m_providerReferences.emplace_back(&provider);
}

bool ResourceManager::TryGetBlob(const ResourceKey& key, ResourceBlob& outBlob)
{
    for (const auto& provider : m_providers) {
        if (provider && provider->TryGet(key, outBlob)) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Debug,
                "[ResourceManager] Resolved resource '%s' via provider %p (%zu bytes)",
                key.Name(), provider.get(), outBlob.Size);
            return true;
        }
    }

    for (const auto& provider : m_providerReferences) {
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
    if (!NeoWPF::Platform::DecodeImageFromMemory(std::vector<uint8_t>(blob.Data, blob.Data + blob.Size), pixels, width, height, 0, 0)) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
            "[ResourceManager] DecodeImageFromMemory failed for '%s' (%zu bytes)", key.Name(), blob.Size);
        return nullptr;
    }

    auto result = UI::Core::Media::BitmapSource::Create(width, height, std::move(pixels));
    if (result) {
        const auto hash = key.Hash();
        m_decodedCache[hash] = result;
    }
    return result;
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
