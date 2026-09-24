/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoPM/Resources/ResourceEntry.h>
#include <vector>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

namespace NeoPM::Core::Resources {
    struct XamlResourceEntry {
        std::string key;
        std::string path;
        bool operator==(const XamlResourceEntry&) const = default;
    };

    class ResourceManifest {
    public:
        static std::optional<ResourceManifest> Load(const std::filesystem::path& manifestPath);
        const std::vector<ResourceEntry>& GetEntries() const { return m_entries; }
        const std::vector<XamlResourceEntry>& GetXamlEntries() const { return m_xamlEntries; }
        std::vector<ResourceEntry> GetEntriesByType(ResourceType type) const;
        void AddEntry(const ResourceEntry& entry) { m_entries.push_back(entry); }
        void SetXamlEntries(std::vector<XamlResourceEntry> entries) { m_xamlEntries = std::move(entries); }
        void Clear() { m_entries.clear(); m_xamlEntries.clear(); }
        bool Save(const std::filesystem::path& manifestPath) const;
        bool HasEmbeddedResources() const;
        std::vector<ResourceEntry> GetExternalResources() const;
    private:
        std::vector<ResourceEntry> m_entries;
        std::vector<XamlResourceEntry> m_xamlEntries;
        ResourceManifest() = default;
    };
}
