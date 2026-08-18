#pragma once

#include <cstddef>

#include <NeoWPF/UI/Data/ResourceKey.h>

namespace NeoWPF {

enum class ResourceType {
    Image,
    Font,
    Raw
};

struct ResourceBlob {
    const unsigned char* Data = nullptr;
    size_t Size = 0;
    ResourceType Type = ResourceType::Raw;
};

class IResourceProvider {
public:
    virtual ~IResourceProvider() = default;
    virtual bool TryGet(const ResourceKey& key, ResourceBlob& outBlob) = 0;
};

}
