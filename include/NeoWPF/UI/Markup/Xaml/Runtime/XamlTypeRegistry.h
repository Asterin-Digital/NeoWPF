#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <vector>

namespace NeoWPF::UI::Properties {
class DependencyObject;
}

namespace NeoWPF::UI::Markup::Xaml {

enum class XamlTypeRole {
    Element,
    WindowRoot,
    ApplicationRoot,
};

/*
    @summary
    Metadata for a type that can be constructed from XAML.

    @details
    This struct holds factory and content-property information for a single XAML-buildable type.
    Both Visuals and non-Visuals (e.g., RowDefinition, Setter, Trigger) can be registered.
*/
struct XamlTypeInfo {
    /* The runtime type (used for instanceof checks, registry lookups by type, etc.) */
    std::type_index Type;

    /* Factory function: constructs a new instance of this type as a DependencyObject */
    std::function<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>()> Factory;

    /* 
        Name of the content property for this type, e.g., "Children" for panels, "Content" for single-slot containers.
        Empty string if this type takes no default content.
    */
    std::string ContentPropertyName;

    /* 
        If true, content is not built eagerly into live objects (e.g., ControlTemplate).
        Instead, the unbuilt node or a factory closure is stored for deferred instantiation.
        Default: false (eager build).
    */
    bool DeferContentBuild = false;

    /* Fully qualified C++ type name used by offline XAML code generation. */
    std::string CppTypeName;

    /* Public include path required to use CppTypeName in generated code. */
    std::string HeaderPath;

    /* Identifies special document-root types for offline code generation. */
    XamlTypeRole Role = XamlTypeRole::Element;
};

struct RegisteredXamlType {
    std::string TagName;
    XamlTypeInfo Info;
};

/*
    @summary
    Central registry for XAML-buildable types.

    @details
    Maps XAML tag names (e.g., "Button", "Grid", "RowDefinition") to XamlTypeInfo metadata.
    Replaces the hardcoded factory map in XamlTreeBuilder.
    
    Both concrete and abstract types can be registered. Lookup is by tag name (string) or by type_index.
*/
class XamlTypeRegistry {
public:
    /*
        @summary
        Returns the singleton registry instance.
    */
    static XamlTypeRegistry& Get();

    /*
        @summary
        Registers a type for XAML construction.

        @param tagName
        The XAML tag name (e.g., "Button", "Grid", "RowDefinition").

        @param info
        Construction metadata for this type.

        @remarks
        If the tag is already registered, this is a no-op (existing registration is kept).
    */
    void Register(std::string_view tagName, const XamlTypeInfo& info);

    /*
        @summary
        Finds a type by its XAML tag name.

        @param tagName
        The tag name to look up (e.g., "Button").

        @return
        Pointer to XamlTypeInfo if found; nullptr otherwise.
    */
    const XamlTypeInfo* Find(std::string_view tagName) const;

    /*
        @summary
        Finds a type by its std::type_index.

        @param type
        The runtime type to look up.

        @return
        Pointer to XamlTypeInfo if found; nullptr otherwise.
    */
    const XamlTypeInfo* FindByType(std::type_index type) const;

    std::vector<RegisteredXamlType> GetRegisteredTypes() const;

private:
    XamlTypeRegistry() = default;

    std::unordered_map<std::string, XamlTypeInfo> m_types;
};

bool ValidateXamlRegistrations();

/*
    @summary
    Registers a type for XAML construction.

    @tparam T
    The type to register. Must derive from DependencyObject.

    @param tagName
    The XAML tag name for this type.

    @param contentProperty
    The name of the default content property (e.g., "Children" for panels).
    Empty string if this type takes no default content.

    @remarks
    This is a convenience template that constructs XamlTypeInfo and registers it.
    Can be called multiple times from different *Properties.cpp files.
*/
template<typename T>
inline void RegisterXamlType(std::string_view tagName,
                             std::string_view contentProperty = "",
                             std::string_view cppTypeName = "",
                             std::string_view headerPath = "",
                             XamlTypeRole role = XamlTypeRole::Element)
{
    static_assert(std::is_base_of_v<NeoWPF::UI::Properties::DependencyObject, T>,
                  "Only DependencyObject-derived types can be registered for XAML.");

    XamlTypeInfo info{
        std::type_index(typeid(T)),
        [] { return std::static_pointer_cast<NeoWPF::UI::Properties::DependencyObject>(std::make_shared<T>()); },
        std::string(contentProperty),
        false,
        std::string(cppTypeName),
        std::string(headerPath),
        role
    };

    XamlTypeRegistry::Get().Register(tagName, info);
}

}
