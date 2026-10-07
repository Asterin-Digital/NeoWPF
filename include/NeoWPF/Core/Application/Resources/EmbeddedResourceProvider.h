/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <mutex>
#include <unordered_map>
#include <utility>

#include <NeoWPF/Core/Application/Resources/IResourceProvider.h>

namespace NeoWPF {

class EmbeddedResourceProvider final : public IResourceProvider {
public:
    static EmbeddedResourceProvider& Instance();

    void Register(const ResourceKey& key, ResourceType type, const unsigned char* data, size_t size);
    void Unregister(const ResourceKey& key);
    void Clear();

    bool TryGet(const ResourceKey& key, ResourceBlob& outBlob) override;

private:
    mutable std::mutex m_mutex;
    std::unordered_map<uint64_t, ResourceBlob> m_blobs;
};

}
