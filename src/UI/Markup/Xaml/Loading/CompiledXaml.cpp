#define NEOWPF_ENABLE_RUNTIME_XAML 1
#include <NeoWPF/UI/Markup/Xaml/Loading/CompiledXaml.h>
#include <NeoWPF/UI/Markup/Xaml/Artifacts/NamlArtifact.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlTypeRegistry.h>
#include <NeoWPF/UI/Markup/Xaml/Parsing/XamlMarkupExtension.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Data/Binding.h>
#include <NeoWPF/UI/Data/BindingOperations.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/Core/Application/Resources/ResourceManager.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/IXamlResourceHost.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <any>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
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
    ArtifactTreeBuilder(const NamlArtifact& artifact, XamlNamescope* namescope, std::string& error)
        : m_artifact(artifact)
        , m_namescope(namescope)
        , m_error(error)
    {
    }

    std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> Build()
    {
        const auto root = BuildObject(m_artifact.rootObjectId);
        return root;
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
        if (m_namescope && !m_artifact.objects[objectId].name.empty() &&
            !m_namescope->Register(m_artifact.objects[objectId].name, object)) {
            m_error = "duplicate or invalid NAML object name: " + m_artifact.objects[objectId].name;
            return nullptr;
        }

        if (!AttachLocaleTables(*object, objectId)) {
            return nullptr;
        }

        auto* element = dynamic_cast<NeoWPF::UI::UIElement*>(object.get());
        if (element) {
            m_scopes.push_back(element);
        }

        for (const auto& child : m_artifact.children) {
            if (child.parentId != objectId ||
                child.propertyString >= m_artifact.strings.size() ||
                m_artifact.strings[child.propertyString] != "Resources") {
                continue;
            }
            auto* resourceHost = dynamic_cast<IXamlResourceHost*>(object.get());
            if (!AttachResource(resourceHost, child)) {
                if (element) m_scopes.pop_back();
                return nullptr;
            }
        }

        for (const auto& property : m_artifact.properties) {
            if (property.objectId != objectId) continue;
            if (!ApplyProperty(*object, property)) return nullptr;
        }
        for (const auto& binding : m_artifact.bindings) {
            if (binding.objectId != objectId) continue;
            if (!ApplyBinding(*object, binding)) return nullptr;
        }
        for (const auto& locale : m_artifact.localeReferences) {
            if (locale.objectId != objectId) continue;
            if (!ApplyLocaleReference(*object, locale)) return nullptr;
        }
        for (const auto& child : m_artifact.children) {
            if (child.parentId != objectId ||
                (child.propertyString < m_artifact.strings.size() &&
                 m_artifact.strings[child.propertyString] == "Resources")) continue;
            if (!AttachChild(*object, child)) return nullptr;
        }

        if (element) {
            m_scopes.pop_back();
        }
        return object;
    }

    bool AttachResource(IXamlResourceHost* owner, const NamlChildRecord& record)
    {
        if (!owner) {
            m_error = "NAML resources owner does not support XAML resources";
            return false;
        }

        const auto resource = BuildObject(record.childId);
        if (!resource) return false;

        const auto& key = m_artifact.objects[record.childId].resourceKey;
        if (key.empty()) {
            const auto style = std::dynamic_pointer_cast<NeoWPF::UI::Styling::Style>(resource);
            if (!style || style->GetTargetType() == std::type_index(typeid(void))) {
                m_error = "NAML resource object is missing a key";
                return false;
            }
            if (!m_implicitStyleTypes.insert(std::make_pair(owner, style->GetTargetType())).second) {
                m_error = "duplicate NAML implicit style target type";
                return false;
            }
            if (!owner->SetImplicitXamlStyle(style->GetTargetType(), style)) {
                m_error = "NAML implicit style could not be stored";
                return false;
            }
            return true;
        }
        if (!m_resourceKeys.insert(std::make_pair(owner, key)).second) {
            m_error = "duplicate NAML resource key: " + key;
            return false;
        }

        if (auto resourceElement = std::dynamic_pointer_cast<NeoWPF::UI::UIElement>(resource)) {
            owner->SetXamlResource(key, resourceElement);
        } else {
            owner->SetXamlResource(key, resource);
        }
        return true;
    }

    bool ApplyProperty(DependencyObject& object, const NamlPropertyRecord& record)
    {
        if (record.propertyString >= m_artifact.strings.size()) {
            m_error = "NAML property reference is invalid";
            return false;
        }
        const auto& propertyName = m_artifact.strings[record.propertyString];
        const auto* property = FindProperty(object, propertyName);
        if (!property) {
            m_error = "NAML property is not registered: " + propertyName;
            return false;
        }
        if (record.value.kind == NeoNaml::NamlValueKind::LocaleReference) {
            if (!NeoWPF::UI::Localization::LocaleOperations::SetLocale(
                    object,
                    *property,
                    NeoWPF::Platform::Utf8ToWide(record.value.text))) {
                m_error = "compiled Locale could not be attached: " + record.value.text;
                return false;
            }
            return true;
        }
        std::any parsedValue;
        if (record.value.kind == NeoNaml::NamlValueKind::MarkupExtension &&
            !record.value.text.empty()) {
            const auto extensionResult = ParseXamlMarkupExtension(record.value.text, {});
            if (extensionResult.Fail()) {
                m_error = extensionResult.Error();
                return false;
            }
            const auto& extension = extensionResult.Value();

            std::string localeKey;
            if (TryGetXamlLocaleKey(*extension, localeKey)) {
                if (!NeoWPF::UI::Localization::LocaleOperations::SetLocale(
                        object,
                        *property,
                        NeoWPF::Platform::Utf8ToWide(localeKey))) {
                    m_error = "compiled Locale could not be attached: " + localeKey;
                    return false;
                }
                return true;
            }

            std::string resourceKey;
            if (!TryGetXamlStaticResource(*extension, resourceKey)) {
                m_error = "unsupported compiled markup extension: " + record.value.text;
                return false;
            }

            if (!ResolveResource(resourceKey, parsedValue)) {
                m_error = "compiled StaticResource was not found: " + resourceKey;
                return false;
            }

            if (const auto* binding = property->GetXamlBinding()) {
                auto resourceObject = std::any_cast<std::shared_ptr<DependencyObject>>(&parsedValue);
                if (!resourceObject) {
                    auto resourceElement = std::any_cast<std::shared_ptr<NeoWPF::UI::UIElement>>(&parsedValue);
                    if (resourceElement) {
                        binding->Assign(object, *resourceElement);
                        return true;
                    }
                } else {
                    binding->Assign(object, *resourceObject);
                    return true;
                }
            }
            object.SetValue(*property, parsedValue);
            return true;
        }
        if (!property->TryParseValue(record.value.text, parsedValue)) {
            m_error = "NAML property value could not be converted: " + propertyName;
            return false;
        }
        object.SetValue(*property, parsedValue);
        return true;
    }

    bool ApplyLocaleReference(DependencyObject& object,
                              const NamlLocaleReferenceRecord& record)
    {
        if (record.propertyString >= m_artifact.strings.size()) {
            m_error = "NAML locale property reference is invalid";
            return false;
        }
        const auto& propertyName = m_artifact.strings[record.propertyString];
        const auto* property = object.FindDependencyProperty(propertyName);
        if (!property) {
            m_error = "NAML locale property is not registered: " + propertyName;
            return false;
        }
        if (!NeoWPF::UI::Localization::LocaleOperations::SetLocale(
                object,
                *property,
                NeoWPF::Platform::Utf8ToWide(record.key))) {
            m_error = "NAML Locale could not be attached: " + propertyName;
            return false;
        }
        return true;
    }

    bool AttachLocaleTables(DependencyObject& object, std::uint32_t objectId)
    {
        auto* application = dynamic_cast<NeoWPF::Core::Application*>(&object);
        auto* window = dynamic_cast<NeoWPF::UI::Window*>(&object);
        for (const auto& tableRecord : m_artifact.localeTables) {
            if (tableRecord.objectId != objectId) {
                continue;
            }
            NeoWPF::UI::Localization::LocaleTable table{
                NeoWPF::UI::Localization::Culture(std::wstring(tableRecord.culture.begin(), tableRecord.culture.end()))};
            if (!table.IsValid()) {
                m_error = "NAML locale table has an invalid culture: " + tableRecord.culture;
                return false;
            }
            for (const auto& stringRecord : m_artifact.localeStrings) {
                if (stringRecord.tableId != tableRecord.id ||
                    !table.Add(NeoWPF::Platform::Utf8ToWide(stringRecord.key),
                               NeoWPF::Platform::Utf8ToWide(stringRecord.value))) {
                    if (stringRecord.tableId == tableRecord.id) {
                        m_error = "NAML locale table contains an invalid or duplicate string";
                        return false;
                    }
                }
            }
            if (application) {
                application->AddLocale(std::make_shared<NeoWPF::UI::Localization::LocaleTable>(table));
            } else if (window) {
                window->AddLocale(std::make_shared<NeoWPF::UI::Localization::LocaleTable>(table));
            } else {
                m_error = "NAML locale table owner is not an Application or Window";
                return false;
            }
        }
        return true;
    }

    bool ResolveResource(const std::string& key, std::any& value) const
    {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            if (*it && (*it)->TryFindResource(key, value)) {
                return true;
            }
        }
        return NeoWPF::Core::ApplicationResources::Resources::TryFindResource(key, value);
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

        const auto& serializedPropertyName = m_artifact.strings[record.propertyString];
        auto propertyName = serializedPropertyName;
        if (propertyName == NeoNaml::ImplicitChildrenPropertyName) {
            const auto* typeInfo = XamlTypeRegistry::Get().FindByType(typeid(object));
            if (typeInfo && !typeInfo->ContentPropertyName.empty()) {
                propertyName = typeInfo->ContentPropertyName;
            }
        }

        const auto* property = FindProperty(object, propertyName);
        const auto* binding = property ? property->GetXamlBinding() : nullptr;
        if (binding) {
            binding->Assign(object, child);
            return true;
        }

        if (serializedPropertyName == NeoNaml::ImplicitChildrenPropertyName) {
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

    static const NeoWPF::UI::Properties::DependencyProperty* FindProperty(
        DependencyObject& object,
        const std::string& propertyName)
    {
        return object.FindDependencyProperty(propertyName);
    }

    const NamlArtifact& m_artifact;
    XamlNamescope* m_namescope;
    std::string& m_error;
    std::unordered_map<std::uint32_t, std::shared_ptr<DependencyObject>> m_objects;
    std::vector<NeoWPF::UI::UIElement*> m_scopes;
    std::set<std::pair<IXamlResourceHost*, std::string>> m_resourceKeys;
    std::set<std::pair<IXamlResourceHost*, std::type_index>> m_implicitStyleTypes;
};

}

NeoWPF::Result<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>> LoadCompiledXaml(
    const NeoWPF::ResourceKey& key,
    XamlNamescope* namescope)
{
    std::string error;

    NeoWPF::ResourceBlob blob;
    if (!NeoWPF::ResourceManager::Instance().TryGetBlob(key, blob)) {
        return NeoWPF::Fail<std::string>{"compiled XAML resource was not found: " + std::string(key.Name())};
    }
    if (blob.Type != NeoWPF::ResourceType::Xaml) {
        return NeoWPF::Fail<std::string>{"resource is not a compiled XAML artifact: " + std::string(key.Name())};
    }

    NamlArtifact artifact;
    if (!DeserializeNaml(blob.Data, blob.Size, artifact, error)) {
        return NeoWPF::Fail<std::string>{std::move(error)};
    }

    for (const auto& object : artifact.objects) {
        if (object.id != &object - artifact.objects.data()) {
            return NeoWPF::Fail<std::string>{"NAML object identifiers are not contiguous"};
        }
    }

    ArtifactTreeBuilder builder(artifact, namescope, error);
    auto root = builder.Build();
    if (!root) {
        return NeoWPF::Fail<std::string>{std::move(error)};
    }
    return NeoWPF::Success<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>>{
        std::move(root)};
}

}
