#pragma once

#include <filesystem>

#include <NeoWPF/Core/Application/Resources/IResourceProvider.h>

namespace NeoWPF {

class ExternalFileResourceProvider final : public IResourceProvider {
public:
    explicit ExternalFileResourceProvider(std::filesystem::path baseDirectory);

    bool TryGet(const ResourceKey& key, ResourceBlob& outBlob) override;

private:
    std::filesystem::path m_baseDirectory;
};

}
