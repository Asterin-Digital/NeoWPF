/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace NeoWPF::UI {
    namespace Data {
        class Binding;
        class BindingExpression;
        class BindingOperations;
    }
    namespace Localization {
        class LocaleExpression;
        class LocaleOperations;
    }
}

namespace NeoWPF::UI::Properties {
    class DependencyProperty;
    enum class PropertyValueSource;
    struct PropertyMetadata;

    /*
        @summary
        Base type for objects that participate in NeoWPF's dependency property system.

        @details
        DependencyObject provides storage for local, style, and binding-based values, along with value precedence and change notification hooks for derived classes.
    */
    class DependencyObject {
    public:
        virtual ~DependencyObject() = default;

        /*
            @summary
            Reads a dependency property value using the requested type.

            @tparam T
            The expected property value type.

            @param property
            The dependency property to read.

            @returns
            The current value converted to the requested type.
        */
        template<typename T>
        T GetValue(const DependencyProperty& property) const;

        template<typename T>
        T GetValue(const std::string& propertyName) const
        {
            if (const auto* property = FindDependencyProperty(propertyName)) {
                return GetValue<T>(*property);
            }
            return T{};
        }

        /*
            @summary
            Sets a dependency property value using the provided strongly typed value.

            @tparam T
            The property value type.

            @param property
            The dependency property to update.

            @param value
            The new value for the property.
        */
        template<typename T>
        void SetValue(const DependencyProperty& property, const T& value);

        template<typename T>
        void SetValue(const std::string& propertyName, const T& value)
        {
            if (const auto* property = FindDependencyProperty(propertyName)) {
                SetValue(*property, value);
            }
        }

        /*
            @summary
            Sets a dependency property value from an std::any container.

            @param property
            The dependency property to update.

            @param value
            The value to store for the property.
        */
        void SetValue(const DependencyProperty& property, const std::any& value);
        void SetValue(const DependencyProperty* property, const std::any& value);

        /*
            @summary
            Associates an expression-based value with a dependency property.

            @param property
            The dependency property to update.

            @param value
            The binding expression or expression payload to assign.
        */
        void SetValueExpression(const DependencyProperty& property, const std::any& value);
        void SetValueExpression(const DependencyProperty* property, const std::any& value);

        void SetValueAnimated(const DependencyProperty& property, const std::any& value);
        void ClearAnimatedValue(const DependencyProperty& property);

        /*
            @summary
            Attaches a binding to a dependency property using the common target-side convenience API.
        */
        void SetBinding(const DependencyProperty& property, const Data::Binding& binding);
        void SetBinding(const DependencyProperty* property, const Data::Binding& binding);
        void SetBinding(const std::string& propertyName, const Data::Binding& binding);

        /*
            @summary
            Finds a dependency property by name for this object.

            @param propertyName
            The property name to look up.

            @returns
            The dependency property definition, or nullptr if it does not exist.
            
        */
        const DependencyProperty* FindDependencyProperty(const std::string& propertyName) const;

        /*
            @summary
            Clears any binding currently attached to a dependency property.
        */
        void ClearBinding(const DependencyProperty& property);
        void ClearBinding(const DependencyProperty* property);
        void ClearBinding(const std::string& propertyName);

        /*
            @summary
            Clears the local value for a dependency property.

            @param property
            The dependency property whose local value should be removed.
        */
        void ClearValue(const DependencyProperty& property);
        void ClearValue(const DependencyProperty* property);

        /*
            @summary
            Returns the local value for a dependency property when one exists.

            @param property
            The dependency property to inspect.

            @returns
            The local value stored for the property, if any.
        */
        const std::any& ReadLocalValue(const DependencyProperty& property) const;
        const std::any& ReadLocalValue(const DependencyProperty* property) const;

        /*
            @summary
            Indicates where the current value for a property originated.

            @param property
            The dependency property to inspect.

            @returns
            The precedence source for the property value.
        */
        PropertyValueSource GetValueSource(const DependencyProperty& property) const;
        PropertyValueSource GetValueSource(const DependencyProperty* property) const;

        /*
            @summary
            Returns the raw value stored for a dependency property.

            @param property
            The dependency property to inspect.

            @returns
            The current raw value for the property.
        */
        std::any GetRawValue(const DependencyProperty& property) const;
        std::any GetRawValue(const DependencyProperty* property) const;

    protected:
        /*
            @summary
            Returns whether the object is currently applying a property update.

            @returns
            True while a dependent property update is in progress.
        */
        bool IsApplyingPropertyValue() const noexcept { return m_isApplyingPropertyValue; }

        bool IsXamlComponentInitialized() const noexcept { return m_isXamlComponentInitialized; }
        void MarkXamlComponentInitialized() noexcept { m_isXamlComponentInitialized = true; }

        /*
            @summary
            Returns the object to use as the inheritance parent for inherited property values.

            @returns
            The dependency object to use for inheritance lookup.
        */
        virtual const DependencyObject* GetInheritanceParent() const noexcept { return nullptr; }

        /*
            @summary
            Applies a value update to a dependency property.

            @param property
            The dependency property being updated.

            @param value
            The new value to apply.

            @param source
            The precedence source of the update.
        */
        void SetValueInternal(const DependencyProperty& property,
                              std::any value,
                              PropertyValueSource source);

        /*
            @summary
            Invoked when a dependency property changes value.

            @param property
            The dependency property that changed.

            @param metadata
            The metadata associated with the property.

            @param oldValue
            The previous property value.

            @param newValue
            The new property value.

            @param source
            The source that produced the new value.
        */
        virtual void OnDependencyPropertyChanged(const DependencyProperty& property,
                                                 const PropertyMetadata& metadata,
                                                 const std::any& oldValue,
                                                 const std::any& newValue,
                                                 PropertyValueSource source) {}

    public:
        void RefreshBindingExpressions();

    private:
        std::any GetRawValueNoLock(const DependencyProperty& property) const;
        friend class ::NeoWPF::UI::Data::BindingOperations;
        friend class ::NeoWPF::UI::Data::BindingExpression;
        friend class ::NeoWPF::UI::Localization::LocaleOperations;

        static std::optional<std::any> FindRawValue(const DependencyObject* obj,
                                                    const DependencyProperty& property);

        void SetBindingExpression(const DependencyProperty& property,
                                  std::shared_ptr<::NeoWPF::UI::Data::BindingExpression> expression);
        std::shared_ptr<::NeoWPF::UI::Data::BindingExpression> GetBindingExpression(const DependencyProperty& property) const;
        void ClearBindingExpression(const DependencyProperty& property);

        void SetLocaleExpression(
            const DependencyProperty& property,
            std::shared_ptr<::NeoWPF::UI::Localization::LocaleExpression> expression);
        std::shared_ptr<::NeoWPF::UI::Localization::LocaleExpression> GetLocaleExpression(
            const DependencyProperty& property) const;
        void ClearLocaleExpression(const DependencyProperty& property);
        void ClearLocaleExpressionValue(const DependencyProperty& property);

        void NotifyDependencyPropertyChanged(const DependencyProperty& property,
                              const std::any& oldValue,
                              const std::any& newValue,
                              PropertyValueSource source);

        std::size_t AddDependencyPropertyChangedHandler(const DependencyProperty& property,
                                                        std::function<void(const std::any&, const std::any&)> handler);
        void RemoveDependencyPropertyChangedHandler(const DependencyProperty& property,
                                                    std::size_t handlerId);

        bool m_isApplyingPropertyValue = false;
        bool m_isXamlComponentInitialized = false;
        std::unordered_map<const DependencyProperty*, std::any> m_expressionValues;
        std::unordered_map<const DependencyProperty*, std::any> m_localValues;
        std::unordered_map<const DependencyProperty*, std::any> m_themeValues;
        std::unordered_map<const DependencyProperty*, std::any> m_styleValues;
        std::unordered_map<const DependencyProperty*, std::any> m_triggerValues;
        std::unordered_map<const DependencyProperty*, std::any> m_animatedValues;
        std::unordered_map<const DependencyProperty*, std::unordered_map<std::size_t, std::function<void(const std::any&, const std::any&)>>> m_dependencyPropertyChangedHandlers;
        std::size_t m_nextDependencyPropertyChangedHandlerId = 1;
        std::unordered_map<const DependencyProperty*, std::shared_ptr<::NeoWPF::UI::Data::BindingExpression>> m_bindingExpressions;
        std::unordered_map<const DependencyProperty*, std::shared_ptr<::NeoWPF::UI::Localization::LocaleExpression>> m_localeExpressions;
    };

    template<typename T>
    T DependencyObject::GetValue(const DependencyProperty& property) const
    {
        return std::any_cast<T>(GetRawValue(property));
    }

    template<typename T>
    void DependencyObject::SetValue(const DependencyProperty& property, const T& value)
    {
        if constexpr (TypeTraits::IsAnyEqualityComparable<T>::value) {
            RegisterAnyEqualityComparer<T>();
        }
        SetValue(property, std::any(value));
    }

}
