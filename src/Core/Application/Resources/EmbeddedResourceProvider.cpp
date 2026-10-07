#include <NeoWPF/Core/Application/Resources/EmbeddedResourceProvider.h>

#include <vector>

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

    auto owner = std::make_shared<std::vector<unsigned char>>(data, data + size);
    ResourceBlob blob{};
    blob.Data = owner->data();
    blob.Size = owner->size();
    blob.Type = type;
    blob.Owner = owner;

    std::lock_guard lock(m_mutex);
    m_blobs[key.Hash()] = blob;
}

void EmbeddedResourceProvider::Unregister(const ResourceKey& key)
{
    std::lock_guard lock(m_mutex);
    m_blobs.erase(key.Hash());
}

void EmbeddedResourceProvider::Clear()
{
    std::lock_guard lock(m_mutex);
    m_blobs.clear();
}

bool EmbeddedResourceProvider::TryGet(const ResourceKey& key, ResourceBlob& outBlob)
{
    std::lock_guard lock(m_mutex);
    const auto it = m_blobs.find(key.Hash());
    if (it == m_blobs.end()) {
        return false;
    }

    outBlob = it->second;
    return true;
}

}
