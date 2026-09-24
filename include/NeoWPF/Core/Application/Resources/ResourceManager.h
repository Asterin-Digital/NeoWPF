/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <NeoWPF/UI/Core/Media/BitmapSource.h>
#include <NeoWPF/Core/Application/Resources/IResourceProvider.h>

struct IDWriteFontFace;

namespace NeoWPF {

class ResourceManager {
public:
    static ResourceManager& Instance();

    void RegisterProvider(std::unique_ptr<IResourceProvider> provider);
    void RegisterProviderReference(IResourceProvider& provider);
    bool TryGetBlob(const ResourceKey& key, ResourceBlob& outBlob);

    std::shared_ptr<UI::Core::Media::BitmapSource> FindImage(const ResourceKey& key);
    std::shared_ptr<IDWriteFontFace> FindFont(const ResourceKey& key);
    ResourceBlob FindRaw(const ResourceKey& key);

private:
    std::vector<std::unique_ptr<IResourceProvider>> m_providers;
    std::vector<IResourceProvider*> m_providerReferences;
    std::unordered_map<uint64_t, std::shared_ptr<void>> m_decodedCache;
};

}
