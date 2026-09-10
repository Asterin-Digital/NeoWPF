#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace NeoNaml {

inline constexpr std::string_view ImplicitChildrenPropertyName = "Children";

enum class NamlValueKind : std::uint8_t {
    String,
    Integer,
    FloatingPoint,
    Boolean,
    Null,
    MarkupExtension,
    LocaleReference
};

struct NamlValue {
    NamlValueKind kind = NamlValueKind::String;
    std::string text;
};

struct NamlObjectRecord {
    std::uint32_t id = 0;
    std::uint32_t typeString = 0;
    std::string name;
    std::string resourceKey;
};

struct NamlPropertyRecord {
    std::uint32_t objectId = 0;
    std::uint32_t propertyString = 0;
    NamlValue value;
};

struct NamlChildRecord {
    std::uint32_t parentId = 0;
    std::uint32_t childId = 0;
    std::uint32_t propertyString = 0;
};

struct NamlBindingRecord {
    std::uint32_t objectId = 0;
    std::uint32_t propertyString = 0;
    std::string path;
    std::string mode;
};

struct NamlLocaleReferenceRecord {
    std::uint32_t objectId = 0;
    std::uint32_t propertyString = 0;
    std::string key;
};

struct NamlLocaleTableRecord {
    std::uint32_t id = 0;
    std::uint32_t objectId = 0;
    std::string culture;
};

struct NamlLocaleStringRecord {
    std::uint32_t tableId = 0;
    std::string key;
    std::string value;
};

struct NamlArtifact {
    static constexpr std::uint32_t CurrentVersion = 1;
    static constexpr std::uint32_t MinimumSupportedVersion = 1;

    std::string resourceKey;
    std::string sourcePath;
    std::uint32_t rootObjectId = 0;
    std::vector<std::string> strings;
    std::vector<NamlObjectRecord> objects;
    std::vector<NamlPropertyRecord> properties;
    std::vector<NamlChildRecord> children;
    std::vector<NamlBindingRecord> bindings;
    std::vector<NamlLocaleReferenceRecord> localeReferences;
    std::vector<NamlLocaleTableRecord> localeTables;
    std::vector<NamlLocaleStringRecord> localeStrings;
};

std::string SerializeNaml(const NamlArtifact& artifact);

bool DeserializeNaml(const unsigned char* data,
                     std::size_t size,
                     NamlArtifact& artifact,
                     std::string& error);

}
