#include <NeoWPF/Core/Application/Resources/EmbeddedResourceProvider.h>

namespace NeoWPF {

EmbeddedResourceProvider& EmbeddedResourceProvider::Instance()
{
    static EmbeddedResourceProvider provider;
    return provider;
}

void EmbeddedResourceProvider::Register(const ResourceKey& key, ResourceType type, const unsigned char* data, size_t size)
{
    if (!data || size == 0) {
        return;
    }

    ResourceBlob blob{};
    blob.Data = data;
    blob.Size = size;
    blob.Type = type;
    m_blobs[key.Hash()] = blob;
}

void EmbeddedResourceProvider::Unregister(const ResourceKey& key)
{
    m_blobs.erase(key.Hash());
}

void EmbeddedResourceProvider::Clear()
{
    m_blobs.clear();
}

bool EmbeddedResourceProvider::TryGet(const ResourceKey& key, ResourceBlob& outBlob)
{
    const auto it = m_blobs.find(key.Hash());
    if (it == m_blobs.end()) {
        return false;
    }

    outBlob = it->second;
    return true;
}

}
