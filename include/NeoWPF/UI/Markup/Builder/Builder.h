/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Markup/Xaml/Runtime/XamlPropertyBinding.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace NeoWPF::UI::Builder {

template<typename T>
class ElementBuilder {
public:
    static_assert(std::is_base_of_v<NeoWPF::UI::Properties::DependencyObject, T>,
        "ElementBuilder<T> requires T to derive from DependencyObject.");

    ElementBuilder()
        : m_element(std::make_shared<T>())
    {
    }

    explicit ElementBuilder(std::shared_ptr<T> element)
        : m_element(std::move(element))
    {
    }

    template<typename TValue>
    ElementBuilder& Set(const NeoWPF::UI::Properties::DependencyProperty& property, const TValue& value)
    {
        if (m_element) {
            m_element->SetValue(property, value);
        }
        return *this;
    }

    template<typename TValue>
    ElementBuilder& Set(const NeoWPF::UI::Properties::DependencyProperty& property, TValue&& value)
    {
        if (m_element) {
            m_element->SetValue(property, std::forward<TValue>(value));
        }
        return *this;
    }

    template<typename U>
    ElementBuilder& AddChild(const std::shared_ptr<U>& child)
    {
        static_assert(std::is_base_of_v<NeoWPF::UI::Visual, U>,
            "Builder AddChild requires a Visual-derived object.");

        if (m_element && child) {
            m_element->AddChild(child);
        }
        return *this;
    }

    template<typename U>
    ElementBuilder& Assign(const NeoWPF::UI::Properties::DependencyProperty& property,
                           const std::shared_ptr<U>& child)
    {
        static_assert(std::is_base_of_v<NeoWPF::UI::Properties::DependencyObject, U>,
            "Builder Assign requires a DependencyObject-derived object.");

        if (m_element && child) {
            const auto* binding = NeoWPF::UI::Markup::Xaml::XamlPropertyBindingRegistry::Get().Find(property);
            if (binding && binding->Assign) {
                binding->Assign(*m_element, child);
            }
        }
        return *this;
    }

    template<typename... ChildTs>
    ElementBuilder& Children(ChildTs&&... children)
    {
        (AppendChild(std::forward<ChildTs>(children)), ...);
        return *this;
    }

    template<typename EventMemberPtr, typename Handler>
    ElementBuilder& On(EventMemberPtr eventMember, Handler&& handler)
    {
        static_assert(std::is_member_pointer_v<EventMemberPtr>,
            "Builder::On requires a pointer-to-member event field.");

        if (m_element && eventMember) {
            auto& event = m_element.get()->*eventMember;
            event += std::forward<Handler>(handler);
        }
        return *this;
    }

    ElementBuilder& Name(const std::string& name)
    {
        m_name = name;
        if (m_element) {
            if constexpr (requires { m_element->SetName(name); }) {
                m_element->SetName(name);
            }
        }
        return *this;
    }

    const std::string& GetName() const noexcept
    {
        return m_name;
    }

    std::shared_ptr<T> Build() const noexcept
    {
        return m_element;
    }

    T* operator->() const noexcept
    {
        return m_element.get();
    }

    operator bool() const noexcept
    {
        return static_cast<bool>(m_element);
    }

    operator T*() const noexcept
    {
        return m_element.get();
    }

    operator std::shared_ptr<T>() const noexcept
    {
        return m_element;
    }

private:
    template<typename U>
    void AppendChild(const ElementBuilder<U>& child)
    {
        if (child) {
            AddChild(child.Build());
        }
    }

    template<typename U>
    void AppendChild(ElementBuilder<U>&& child)
    {
        if (child) {
            AddChild(std::move(child.Build()));
        }
    }

    template<typename U>
    void AppendChild(const std::shared_ptr<U>& child)
    {
        AddChild(child);
    }

    std::shared_ptr<T> m_element;
    std::string m_name;
};

template<typename T>
ElementBuilder<T> Build()
{
    return ElementBuilder<T>{};
}

}
