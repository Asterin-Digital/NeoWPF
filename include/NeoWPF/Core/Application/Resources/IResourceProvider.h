/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstddef>
#include <memory>

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
    std::shared_ptr<const void> Owner;
};

class IResourceProvider {
public:
    virtual ~IResourceProvider() = default;
    virtual bool TryGet(const ResourceKey& key, ResourceBlob& outBlob) = 0;
};

}
