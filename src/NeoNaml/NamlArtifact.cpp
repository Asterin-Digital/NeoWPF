#include <NeoNaml/NamlArtifact.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <charconv>
#include <sstream>
#include <string_view>

namespace NeoNaml {
namespace {

const char* ToString(NamlValueKind kind)
{
    switch (kind) {
    case NamlValueKind::String: return "string";
    case NamlValueKind::Integer: return "integer";
    case NamlValueKind::FloatingPoint: return "float";
    case NamlValueKind::Boolean: return "boolean";
    case NamlValueKind::Null: return "null";
    case NamlValueKind::MarkupExtension: return "markup-extension";
    case NamlValueKind::LocaleReference: return "locale-reference";
    }
    return "unknown";
}

std::string Escape(std::string_view value)
{
    std::string result;
    result.reserve(value.size());
    for (const char character : value) {
        switch (character) {
        case '\\': result += "\\\\"; break;
        case '"': result += "\\\""; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result.push_back(character); break;
        }
    }
    return result;
}

bool ParseUnsigned(std::string_view value, std::uint32_t& result)
{
    const auto trimmed = NeoWPF::Platform::Trim(std::string(value));
    const auto parsed = std::from_chars(trimmed.data(), trimmed.data() + trimmed.size(), result);
    return parsed.ec == std::errc{} && parsed.ptr == trimmed.data() + trimmed.size();
}

bool QuotedValue(std::string_view line, std::string_view key, std::string& result)
{
    const auto prefix = line.find(key);
    if (prefix == std::string_view::npos) return false;
    const auto start = line.find('"', prefix + key.size());
    if (start == std::string_view::npos) return false;

    result.clear();
    bool escaped = false;
    for (auto index = start + 1; index < line.size(); ++index) {
        const char character = line[index];
        if (escaped) {
            switch (character) {
            case 'n': result.push_back('\n'); break;
            case 'r': result.push_back('\r'); break;
            case 't': result.push_back('\t'); break;
            case '\\': result.push_back('\\'); break;
            case '"': result.push_back('"'); break;
            default: result.push_back(character); break;
            }
            escaped = false;
        } else if (character == '\\') {
            escaped = true;
        } else if (character == '"') {
            return true;
        } else {
            result.push_back(character);
        }
    }
    return false;
}

std::string Field(std::string_view line, std::string_view name)
{
    const auto prefix = line.find(std::string(name) + "=");
    if (prefix == std::string_view::npos) return {};
    const auto start = prefix + name.size() + 1;
    const auto end = line.find(' ', start);
    return NeoWPF::Platform::Trim(std::string(line.substr(
        start, end == std::string_view::npos ? line.size() - start : end - start)));
}

bool ParseKind(std::string_view value, NamlValueKind& kind)
{
    if (value == "string") kind = NamlValueKind::String;
    else if (value == "integer") kind = NamlValueKind::Integer;
    else if (value == "float") kind = NamlValueKind::FloatingPoint;
    else if (value == "boolean") kind = NamlValueKind::Boolean;
    else if (value == "null") kind = NamlValueKind::Null;
    else if (value == "markup-extension") kind = NamlValueKind::MarkupExtension;
    else if (value == "locale-reference") kind = NamlValueKind::LocaleReference;
    else return false;
    return true;
}

bool ParseRecord(std::string_view line, std::string_view section, NamlArtifact& artifact, std::string& error)
{
    const auto trimmed = NeoWPF::Platform::Trim(std::string(line));
    if (trimmed.empty() || trimmed == "}") return true;

    if (section.empty() && trimmed.rfind("version=", 0) == 0) {
        std::uint32_t version = 0;
        if (!ParseUnsigned(trimmed.substr(8), version) ||
            version < NamlArtifact::MinimumSupportedVersion ||
            version > NamlArtifact::CurrentVersion) {
            error = "unsupported NAML version";
            return false;
        }
        return true;
    }
    if (section.empty() && trimmed.rfind("resourceKey=", 0) == 0) return QuotedValue(trimmed, "resourceKey=", artifact.resourceKey);
    if (section.empty() && trimmed.rfind("source=", 0) == 0) return QuotedValue(trimmed, "source=", artifact.sourcePath);
    if (section.empty() && trimmed.rfind("root=", 0) == 0) return ParseUnsigned(trimmed.substr(5), artifact.rootObjectId);

    std::uint32_t id = 0;
    if (section == "strings") {
        const auto equals = trimmed.find(" = ");
        if (equals == std::string_view::npos || !ParseUnsigned(trimmed.substr(0, equals), id)) return false;
        std::string value;
        if (!QuotedValue(trimmed.substr(equals + 3), "", value)) return false;
        if (artifact.strings.size() <= id) artifact.strings.resize(id + 1);
        artifact.strings[id] = std::move(value);
        return true;
    }

    if (section == "objects") {
        const auto colon = trimmed.find(':');
        if (colon == std::string_view::npos || !ParseUnsigned(trimmed.substr(0, colon), id)) return false;
        std::uint32_t type = 0;
        if (!ParseUnsigned(Field(trimmed.substr(colon + 1), "type"), type)) return false;
        NamlObjectRecord object{id, type, {}};
        if (trimmed.find("name=") != std::string_view::npos && !QuotedValue(trimmed, "name=", object.name)) return false;
        if (trimmed.find("resourceKey=") != std::string_view::npos &&
            !QuotedValue(trimmed, "resourceKey=", object.resourceKey)) return false;
        if (artifact.objects.size() <= id) artifact.objects.resize(id + 1);
        artifact.objects[id] = std::move(object);
        return true;
    }

    if (section == "properties") {
        NamlPropertyRecord property;
        if (!ParseUnsigned(Field(trimmed, "object"), property.objectId) ||
            !ParseUnsigned(Field(trimmed, "property"), property.propertyString) ||
            !QuotedValue(trimmed, "value=", property.value.text) ||
            !ParseKind(Field(trimmed, "kind"), property.value.kind)) {
            error = "malformed NAML property record";
            return false;
        }
        artifact.properties.push_back(std::move(property));
        return true;
    }

    if (section == "children") {
        NamlChildRecord child;
        if (!ParseUnsigned(Field(trimmed, "parent"), child.parentId) ||
            !ParseUnsigned(Field(trimmed, "child"), child.childId) ||
            !ParseUnsigned(Field(trimmed, "property"), child.propertyString)) return false;
        artifact.children.push_back(child);
        return true;
    }

    if (section == "bindings") {
        NamlBindingRecord binding;
        if (!ParseUnsigned(Field(trimmed, "object"), binding.objectId) ||
            !ParseUnsigned(Field(trimmed, "property"), binding.propertyString) ||
            !QuotedValue(trimmed, "mode=", binding.mode) ||
            !QuotedValue(trimmed, "path=", binding.path)) return false;
        artifact.bindings.push_back(std::move(binding));
        return true;
    }

    if (section == "locale-references") {
        NamlLocaleReferenceRecord reference;
        if (!ParseUnsigned(Field(trimmed, "object"), reference.objectId) ||
            !ParseUnsigned(Field(trimmed, "property"), reference.propertyString) ||
            !QuotedValue(trimmed, "key=", reference.key) || reference.key.empty()) {
            error = "malformed NAML locale reference record";
            return false;
        }
        artifact.localeReferences.push_back(std::move(reference));
        return true;
    }

    if (section == "locale-tables") {
        NamlLocaleTableRecord table;
        if (!ParseUnsigned(Field(trimmed, "id"), table.id) ||
            !ParseUnsigned(Field(trimmed, "object"), table.objectId) ||
            !QuotedValue(trimmed, "culture=", table.culture) || table.culture.empty()) {
            error = "malformed NAML locale table record";
            return false;
        }
        if (artifact.localeTables.size() <= table.id) artifact.localeTables.resize(table.id + 1);
        artifact.localeTables[table.id] = std::move(table);
        return true;
    }

    if (section == "locale-strings") {
        NamlLocaleStringRecord string;
        if (!ParseUnsigned(Field(trimmed, "table"), string.tableId) ||
            !QuotedValue(trimmed, "key=", string.key) || string.key.empty() ||
            !QuotedValue(trimmed, "value=", string.value)) {
            error = "malformed NAML locale string record";
            return false;
        }
        artifact.localeStrings.push_back(std::move(string));
        return true;
    }

    error = "malformed NAML record in section: " + std::string(section);
    return false;
}

}

std::string SerializeNaml(const NamlArtifact& artifact)
{
    std::ostringstream output;
    output << "NAML1\n";
    output << "version=" << NamlArtifact::CurrentVersion << "\n";
    output << "resourceKey=\"" << Escape(artifact.resourceKey) << "\"\n";
    output << "source=\"" << Escape(artifact.sourcePath) << "\"\n";
    output << "root=" << artifact.rootObjectId << "\n\n";

    output << "strings {\n";
    for (std::size_t index = 0; index < artifact.strings.size(); ++index) {
        output << "  " << index << " = \"" << Escape(artifact.strings[index]) << "\"\n";
    }
    output << "}\n\nobjects {\n";
    for (const auto& object : artifact.objects) {
        output << "  " << object.id << ": type=" << object.typeString;
        if (!object.name.empty()) output << " name=\"" << Escape(object.name) << "\"";
        if (!object.resourceKey.empty()) output << " resourceKey=\"" << Escape(object.resourceKey) << "\"";
        output << "\n";
    }
    output << "}\n\nproperties {\n";
    for (const auto& property : artifact.properties) {
        output << "  object=" << property.objectId << " property=" << property.propertyString
               << " kind=" << ToString(property.value.kind)
               << " value=\"" << Escape(property.value.text) << "\"\n";
    }
    output << "}\n\nchildren {\n";
    for (const auto& child : artifact.children) {
        output << "  parent=" << child.parentId << " child=" << child.childId
               << " property=" << child.propertyString << "\n";
    }
    output << "}\n\nbindings {\n";
    for (const auto& binding : artifact.bindings) {
        output << "  object=" << binding.objectId << " property=" << binding.propertyString
               << " mode=\"" << Escape(binding.mode) << "\" path=\"" << Escape(binding.path) << "\"\n";
    }
    output << "}\n\nlocale-references {\n";
    for (const auto& reference : artifact.localeReferences) {
        output << "  object=" << reference.objectId << " property=" << reference.propertyString
               << " key=\"" << Escape(reference.key) << "\"\n";
    }
    output << "}\n\nlocale-tables {\n";
    for (const auto& table : artifact.localeTables) {
        output << "  id=" << table.id << " object=" << table.objectId
               << " culture=\"" << Escape(table.culture) << "\"\n";
    }
    output << "}\n\nlocale-strings {\n";
    for (const auto& string : artifact.localeStrings) {
        output << "  table=" << string.tableId << " key=\"" << Escape(string.key)
               << "\" value=\"" << Escape(string.value) << "\"\n";
    }
    output << "}\n\nend\n";
    return output.str();
}

bool DeserializeNaml(const unsigned char* data,
                     std::size_t size,
                     NamlArtifact& artifact,
                     std::string& error)
{
    artifact = {};
    error.clear();
    if (!data || size == 0) { error = "NAML artifact is empty"; return false; }

    const std::string content(reinterpret_cast<const char*>(data), size);
    std::istringstream input(content);
    std::string line;
    bool header = false;
    std::string section;
    while (std::getline(input, line)) {
        const auto trimmed = NeoWPF::Platform::Trim(std::string(line));
        if (trimmed == "NAML1") { header = true; continue; }
        if (trimmed.ends_with(" {")) { section = trimmed.substr(0, trimmed.size() - 2); continue; }
        if (trimmed == "end") break;
        if (!ParseRecord(line, section, artifact, error)) {
            if (error.empty()) error = "malformed NAML record: " + trimmed;
            return false;
        }
    }
    if (!header || artifact.objects.empty() || artifact.rootObjectId >= artifact.objects.size()) {
        error = "NAML artifact is missing a valid header or root object";
        return false;
    }
    return true;
}

}
