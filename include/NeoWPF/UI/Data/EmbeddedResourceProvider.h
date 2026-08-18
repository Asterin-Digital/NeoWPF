#pragma once

#include <unordered_map>
#include <utility>

#include <NeoWPF/UI/Data/IResourceProvider.h>

namespace NeoWPF {

class EmbeddedResourceProvider final : public IResourceProvider {
public:
    static EmbeddedResourceProvider& Instance();

    void Register(const ResourceKey& key, ResourceType type, const unsigned char* data, size_t size);
    void Unregister(const ResourceKey& key);
    void Clear();

    bool TryGet(const ResourceKey& key, ResourceBlob& outBlob) override;

private:
    std::unordered_map<uint64_t, ResourceBlob> m_blobs;
};

}
