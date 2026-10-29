/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

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

/*
    @summary
    Identifies the role a registered type plays in a XAML document.
*/
enum class XamlTypeRole {
    Element,
    WindowRoot,
    ApplicationRoot,
};

/*
    @summary
    Stores construction and content metadata for a registered XAML type.
*/
struct XamlTypeInfo {
    std::type_index Type;

    std::function<std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>()> Factory;

    std::string ContentPropertyName;

    bool DeferContentBuild = false;

    std::string CppTypeName;

    std::string HeaderPath;

    XamlTypeRole Role = XamlTypeRole::Element;
};

/*
    @summary
    Associates a XAML tag name with its registered type metadata.
*/
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

    /*
        @summary
        Returns the XAML types registered with the registry.
    */
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
