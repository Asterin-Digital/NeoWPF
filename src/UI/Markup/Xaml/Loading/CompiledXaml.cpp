#define NEOWPF_ENABLE_RUNTIME_XAML 1
#include <NeoWPF/UI/Markup/Xaml/Loading/CompiledXaml.h>
#include <NeoWPF/UI/Markup/Xaml/Artifacts/NamlArtifact.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Data/Binding.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/Core/Application/Resources/ResourceManager.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace NeoWPF::UI::Markup::Xaml {
namespace {

std::string StringAt(const NamlArtifact& artifact, std::uint32_t id)
{
    return id < artifact.strings.size() ? artifact.strings[id] : std::string();
}

bool ValidObject(const NamlArtifact& artifact, std::uint32_t id)
{
    return id < artifact.objects.size() &&
           artifact.objects[id].id == id &&
           artifact.objects[id].typeString < artifact.strings.size();
}

bool ParseBindingMode(const std::string& value, NeoWPF::UI::Data::BindingMode& mode)
{
    if (value.empty() || value == "OneWay") {
        mode = NeoWPF::UI::Data::BindingMode::OneWay;
        return true;
    }
    if (value == "TwoWay") {
        mode = NeoWPF::UI::Data::BindingMode::TwoWay;
        return true;
    }
    if (value == "OneTime") {
        mode = NeoWPF::UI::Data::BindingMode::OneTime;
        return true;
    }
    return false;
}

class ArtifactTreeBuilder {
public:
    ArtifactTreeBuilder(const NamlArtifact& artifact, std::string& error)
        : m_artifact(artifact)
        , m_error(error)
    {
    }

    std::shared_ptr<NeoWPF::UI::Visual> Build()
    {
        const auto root = BuildObject(m_artifact.rootObjectId);
        return std::dynamic_pointer_cast<NeoWPF::UI::Visual>(root);
    }

private:
    using DependencyObject = NeoWPF::UI::Properties::DependencyObject;

    std::shared_ptr<DependencyObject> BuildObject(std::uint32_t objectId)
    {
        const auto existing = m_objects.find(objectId);
        if (existing != m_objects.end()) return existing->second;
        if (!ValidObject(m_artifact, objectId)) {
            m_error = "NAML object reference is invalid";
            return nullptr;
        }

        const auto typeName = StringAt(m_artifact, m_artifact.objects[objectId].typeString);
        const auto* typeInfo = XamlTypeRegistry::Get().Find(typeName);
        if (!typeInfo || !typeInfo->Factory) {
            m_error = "NAML object type is not registered: " + typeName;
            return nullptr;
        }

        auto object = typeInfo->Factory();
        if (!object) {
            m_error = "NAML object could not be constructed: " + typeName;
            return nullptr;
        }
        m_objects.emplace(objectId, object);

        for (const auto& property : m_artifact.properties) {
            if (property.objectId != objectId) continue;
            if (!ApplyProperty(*object, property)) return nullptr;
        }
        for (const auto& binding : m_artifact.bindings) {
            if (binding.objectId != objectId) continue;
            if (!ApplyBinding(*object, binding)) return nullptr;
        }
        for (const auto& child : m_artifact.children) {
            if (child.parentId != objectId) continue;
            if (!AttachChild(*object, child)) return nullptr;
        }
        return object;
    }

    bool ApplyProperty(DependencyObject& object, const NamlPropertyRecord& record)
    {
        if (record.propertyString >= m_artifact.strings.size()) {
            m_error = "NAML property reference is invalid";
            return false;
        }
        const auto& propertyName = m_artifact.strings[record.propertyString];
        const auto* property = object.FindDependencyProperty(propertyName);
        if (!property) {
            m_error = "NAML property is not registered: " + propertyName;
            return false;
        }
        std::any parsedValue;
        if (!property->TryParseValue(record.value.text, parsedValue)) {
            m_error = "NAML property value could not be converted: " + propertyName;
            return false;
        }
        object.SetValue(*property, parsedValue);
        return true;
    }

    bool ApplyBinding(DependencyObject& object, const NamlBindingRecord& record)
    {
        if (record.propertyString >= m_artifact.strings.size()) {
            m_error = "NAML binding property reference is invalid";
            return false;
        }
        const auto& propertyName = m_artifact.strings[record.propertyString];
        const auto* property = object.FindDependencyProperty(propertyName);
        if (!property) {
            m_error = "NAML binding property is not registered: " + propertyName;
            return false;
        }
        NeoWPF::UI::Data::BindingMode mode;
        if (!ParseBindingMode(record.mode, mode)) {
            m_error = "NAML binding mode is invalid: " + record.mode;
            return false;
        }
        if (!NeoWPF::UI::Data::BindingOperations::SetBinding(
                object, *property, NeoWPF::UI::Data::Binding(record.path, mode))) {
            m_error = "NAML binding could not be attached: " + propertyName;
            return false;
        }
        return true;
    }

    bool AttachChild(DependencyObject& object, const NamlChildRecord& record)
    {
        if (record.propertyString >= m_artifact.strings.size()) {
            m_error = "NAML child property reference is invalid";
            return false;
        }
        const auto child = BuildObject(record.childId);
        if (!child) return false;

        const auto& propertyName = m_artifact.strings[record.propertyString];
        const auto* property = propertyName == "Children"
            ? nullptr
            : object.FindDependencyProperty(propertyName);
        const auto* binding = property ? property->GetXamlBinding() : nullptr;
        if (binding) {
            binding->Assign(object, child);
            return true;
        }

        if (propertyName == "Children") {
            auto visualOwner = dynamic_cast<NeoWPF::UI::Visual*>(&object);
            auto visualChild = std::dynamic_pointer_cast<NeoWPF::UI::Visual>(child);
            if (visualOwner && visualChild) {
                visualOwner->AddChild(visualChild);
                return true;
            }
        }

        m_error = "NAML child could not be assigned to property: " + propertyName;
        return false;
    }

    const NamlArtifact& m_artifact;
    std::string& m_error;
    std::unordered_map<std::uint32_t, std::shared_ptr<DependencyObject>> m_objects;
};

}

std::shared_ptr<NeoWPF::UI::Visual> LoadCompiledXaml(const NeoWPF::ResourceKey& key,
                                                     std::string* error)
{
    std::string localError;
    auto& failure = error ? *error : localError;
    failure.clear();

    NeoWPF::ResourceBlob blob;
    if (!NeoWPF::ResourceManager::Instance().TryGetBlob(key, blob)) {
        failure = "compiled XAML resource was not found: " + std::string(key.Name());
        return nullptr;
    }
    if (blob.Type != NeoWPF::ResourceType::Xaml) {
        failure = "resource is not a compiled XAML artifact: " + std::string(key.Name());
        return nullptr;
    }

    NamlArtifact artifact;
    if (!DeserializeNaml(blob.Data, blob.Size, artifact, failure)) return nullptr;

    for (const auto& object : artifact.objects) {
        if (object.id != &object - artifact.objects.data()) {
            failure = "NAML object identifiers are not contiguous";
            return nullptr;
        }
    }

    ArtifactTreeBuilder builder(artifact, failure);
    return builder.Build();
}

}
