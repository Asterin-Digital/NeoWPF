/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace NeoWPF::UI::Properties {
class DependencyObject;
class DependencyProperty;
}

namespace NeoWPF::UI::Markup::Xaml {

/*
    @summary
    Describes how a XAML property should be populated from markup.
*/
enum class XamlPropertyKind {
    Attribute,
    Element,
    Collection
};

/*
    @summary
    Metadata for how a dependency property participates in XAML.

*/
struct XamlPropertyBinding {
    XamlPropertyKind Kind = XamlPropertyKind::Attribute;

    std::string CodeGenerationAdder;

    std::function<void(NeoWPF::UI::Properties::DependencyObject&,
                       std::shared_ptr<NeoWPF::UI::Properties::DependencyObject>)> Assign;
};

/*
    @summary
    Central registry for XAML property bindings.

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
*/
template<typename TOwner, typename TItem>
inline void RegisterXamlCollectionProperty(
    const NeoWPF::UI::Properties::DependencyProperty& property,
    void (TOwner::*adder)(std::shared_ptr<TItem>),
    std::string codeGenerationAdder = {})
{
    XamlPropertyBinding binding{
        XamlPropertyKind::Collection,
        std::move(codeGenerationAdder),
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

*/
template<typename TOwner, typename TItem>
inline void RegisterXamlElementProperty(
    const NeoWPF::UI::Properties::DependencyProperty& property,
    void (TOwner::*setter)(const std::shared_ptr<TItem>&))
{
    XamlPropertyBinding binding{
        XamlPropertyKind::Element,
        {},
        [setter](NeoWPF::UI::Properties::DependencyObject& owner,
                 std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> item) {
            std::shared_ptr<TItem> typedItem;
            if constexpr (std::is_base_of_v<NeoWPF::UI::Properties::DependencyObject, TItem>) {
                typedItem = std::static_pointer_cast<TItem>(std::move(item));
            } else {
                auto* typedItemPointer = dynamic_cast<TItem*>(item.get());
                if (!typedItemPointer) {
                    return;
                }
                typedItem = std::shared_ptr<TItem>(std::move(item), typedItemPointer);
            }
            auto& typedOwner = static_cast<TOwner&>(owner);
            (typedOwner.*setter)(std::move(typedItem));
        }
    };

    XamlPropertyBindingRegistry::Get().Register(property, binding);
}

template<typename TOwner, typename TItem>
inline void RegisterXamlElementPropertyCallback(
    const NeoWPF::UI::Properties::DependencyProperty& property,
    const std::function<void(TOwner&, const std::shared_ptr<TItem>&)>& setter)
{
    XamlPropertyBinding binding{
        XamlPropertyKind::Element,
        {},
        [setter](NeoWPF::UI::Properties::DependencyObject& owner,
                 std::shared_ptr<NeoWPF::UI::Properties::DependencyObject> item) {
            auto typedItem = std::static_pointer_cast<TItem>(std::move(item));
            setter(static_cast<TOwner&>(owner), typedItem);
        }
    };

    XamlPropertyBindingRegistry::Get().Register(property, binding);
}

}
