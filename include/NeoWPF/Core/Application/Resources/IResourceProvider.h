/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>

#include <NeoWPF/Core/Application/Resources/ResourceKey.h>

namespace NeoWPF {

enum class ResourceType {
    Image,
    Font,
    Xaml,
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
