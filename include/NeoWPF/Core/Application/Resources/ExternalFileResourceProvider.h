/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <filesystem>
#include <mutex>

#include <NeoWPF/Core/Application/Resources/IResourceProvider.h>

namespace NeoWPF {

class ExternalFileResourceProvider final : public IResourceProvider {
public:
    explicit ExternalFileResourceProvider(std::filesystem::path baseDirectory);

    bool TryGet(const ResourceKey& key, ResourceBlob& outBlob) override;

private:
    mutable std::mutex m_mutex;
    std::filesystem::path m_baseDirectory;
};

}
