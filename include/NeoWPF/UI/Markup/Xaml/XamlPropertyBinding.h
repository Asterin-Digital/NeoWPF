#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

namespace NeoWPF::UI::Properties {
class DependencyObject;
class DependencyProperty;
}

namespace NeoWPF::UI::Markup::Xaml {

/*
    @summary
    Describes how a XAML property should be populated from markup.

    @details
    Attribute: The property is set via string attribute (e.g., Text="Hello").
    Element: The property is set via a child element (e.g., <Button.Template>...&lt;/Button.Template>).
    Collection: The property is populated by adding multiple child elements (e.g., <Grid.RowDefinitions>...).
*/
enum class XamlPropertyKind {
    Attribute,
    Element,
    Collection
};

/*
    @summary
    Metadata for how a dependency property participates in XAML.

    @details
    Holds the property kind and a typed callback that knows how to assign a built DependencyObject
    to a parent object. The callback contains the only dynamic_cast/static_pointer_cast in this system.
*/
struct XamlPropertyBinding {
    /* The XAML property kind (Attribute, Element, or Collection) */
    XamlPropertyKind Kind = XamlPropertyKind::Attribute;

    /* 
        Callback to assign a built child object to this property on the parent.
        Takes ownership of the child object.
        Never null for Element/Collection kinds.
    */
    std::function<void(NeoWPF::UI::Properties::DependencyObject&,
                       std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>)> Assign;
};

/*
    @summary
    Central registry for XAML property bindings.

    @details
    Maps const DependencyProperty* to XamlPropertyBinding metadata.
    This side-table keeps PropertyMetadata lean (no XAML-specific baggage)
    while providing the infrastructure for property-element and collection-property support.
*/
class XamlPropertyBindingRegistry {
public:
    /*
        @summary
        Returns the singleton registry instance.
    */
    static XamlPropertyBindingRegistry& Get();

    /*
        @summary
        Registers XAML binding metadata for a property.

        @param property
        The dependency property.

        @param binding
        The XAML binding metadata (kind and assignment callback).

        @remarks
        If the property is already registered, this is a no-op (existing binding is kept).
    */
    void Register(const NeoWPF::UI::Properties::DependencyProperty& property, const XamlPropertyBinding& binding);

    /*
        @summary
        Finds XAML binding metadata for a property.

        @param property
        The dependency property.

        @return
        Pointer to XamlPropertyBinding if found; nullptr if this property has no XAML binding.
    */
    const XamlPropertyBinding* Find(const NeoWPF::UI::Properties::DependencyProperty& property) const;

private:
    XamlPropertyBindingRegistry() = default;

    // Key: const DependencyProperty* (pointer value, not dereferenced)
    std::unordered_map<std::uintptr_t, XamlPropertyBinding> m_bindings;
};

/*
    @summary
    Registers a collection-style XAML property.

    @tparam TOwner
    The owner type (e.g., Grid).

    @tparam TItem
    The item type being added (e.g., RowDefinition).

    @param property
    The collection dependency property (e.g., Grid.RowDefinitionsProperty).

    @param adder
    Member function pointer to the adder method (e.g., &Grid::AddRowDefinition).

    @remarks
    This template captures the type information and creates a callback that safely
    casts the generic DependencyObject to TItem and calls the adder.
    The cast failure is NOT handled gracefully (it will crash if types don't match),
    so ensure correct registration.
*/
template<typename TOwner, typename TItem>
inline void RegisterXamlCollectionProperty(
    const NeoWPF::UI::Properties::DependencyProperty& property,
    void (TOwner::*adder)(std::shared_ptr<TItem>))
{
    XamlPropertyBinding binding{
        XamlPropertyKind::Collection,
        [adder](NeoWPF::UI::Properties::DependencyObject& owner,
                std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> item) {
            auto typedItem = std::static_pointer_cast<TItem>(std::move(item));
            auto& typedOwner = static_cast<TOwner&>(owner);
            (typedOwner.*adder)(std::move(typedItem));
        }
    };

    XamlPropertyBindingRegistry::Get().Register(property, binding);
}

/*
    @summary
    Registers a single-element XAML property.

    @tparam TOwner
    The owner type (e.g., Button).

    @tparam TItem
    The item type being set (e.g., ControlTemplate).

    @param property
    The dependency property (e.g., Button.TemplateProperty).

    @param setter
    Member function pointer to the setter method (e.g., &Button::SetTemplate).

    @remarks
    Similar to RegisterXamlCollectionProperty, but for single-slot properties.
*/
template<typename TOwner, typename TItem>
inline void RegisterXamlElementProperty(
    const NeoWPF::UI::Properties::DependencyProperty& property,
    void (TOwner::*setter)(std::shared_ptr<TItem>))
{
    XamlPropertyBinding binding{
        XamlPropertyKind::Element,
        [setter](NeoWPF::UI::Properties::DependencyObject& owner,
                 std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> item) {
            auto typedItem = std::static_pointer_cast<TItem>(std::move(item));
            auto& typedOwner = static_cast<TOwner&>(owner);
            (typedOwner.*setter)(std::move(typedItem));
        }
    };

    XamlPropertyBindingRegistry::Get().Register(property, binding);
}

}
