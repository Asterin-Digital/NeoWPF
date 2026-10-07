#include <NeoWPF/Core/Application/Resources/ExternalFileResourceProvider.h>

#include <NeoWPF/Diagnostics/Logger.h>

#include <fstream>
#include <vector>

namespace NeoWPF {

ExternalFileResourceProvider::ExternalFileResourceProvider(std::filesystem::path baseDirectory)
    : m_baseDirectory(std::move(baseDirectory))
{
}

bool ExternalFileResourceProvider::TryGet(const ResourceKey& key, ResourceBlob& outBlob)
{
    std::lock_guard lock(m_mutex);

    if (m_baseDirectory.empty()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
            "[ExternalFileResourceProvider] Missing base directory for resource '%s'", key.Name());
        return false;
    }

    const auto relativePath = std::filesystem::path(key.Name());
    const std::vector<std::filesystem::path> candidates = {
        m_baseDirectory / "Resources" / relativePath,
        m_baseDirectory / "Resources" / "Assets" / relativePath,
        m_baseDirectory / relativePath
    };

    std::filesystem::path chosenCandidate;
    for (const auto& candidate : candidates) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Debug,
            "[ExternalFileResourceProvider] checking '%ls' for resource '%s'",
            candidate.wstring().c_str(), key.Name());

        if (std::filesystem::exists(candidate)) {
            chosenCandidate = candidate;
            break;
        }
    }

    if (chosenCandidate.empty()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
            "[ExternalFileResourceProvider] Resource '%s' not found under base directory '%ls' (checked Resources, Resources/Assets, and root)",
            key.Name(), m_baseDirectory.wstring().c_str());
        return false;
    }

    if (chosenCandidate.extension() == ".naml") {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
            "[ExternalFileResourceProvider] External XAML resource is not allowed '%s'", key.Name());
        return false;
    }

    std::ifstream file(chosenCandidate, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
            "[ExternalFileResourceProvider] Unable to open resource file '%ls'", chosenCandidate.wstring().c_str());
        return false;
    }

    const auto size = static_cast<size_t>(file.tellg());
    if (size == 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
            "[ExternalFileResourceProvider] Resource file '%ls' is empty", chosenCandidate.wstring().c_str());
        return false;
    }

    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(size))) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
            "[ExternalFileResourceProvider] Failed to read resource file '%ls'", chosenCandidate.wstring().c_str());
        return false;
    }

    const auto owner = std::make_shared<std::vector<unsigned char>>(std::move(buffer));
    outBlob.Data = owner->data();
    outBlob.Size = owner->size();
    outBlob.Type = ResourceType::Raw;
    outBlob.Owner = owner;

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info,
        "[ExternalFileResourceProvider] Loaded resource '%s' from '%ls' (%zu bytes)",
        key.Name(), chosenCandidate.wstring().c_str(), outBlob.Size);
    return true;
}

}
