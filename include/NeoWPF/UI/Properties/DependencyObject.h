#pragma once

#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace NeoWPF::UI {
    namespace Data {
        class BindingExpression;
        class BindingOperations;
    }
}

namespace NeoWPF::UI::Properties {
    class DependencyProperty;
    enum class PropertyValueSource;
    struct PropertyMetadata;

    class DependencyObject {
    public:
        virtual ~DependencyObject() = default;

        template<typename T>
        T GetValue(const DependencyProperty& property) const;

        template<typename T>
        void SetValue(const DependencyProperty& property, const T& value);

        void SetValue(const DependencyProperty& property, const std::any& value);
        void SetValueExpression(const DependencyProperty& property, const std::any& value);

        void ClearValue(const DependencyProperty& property);

        const std::any& ReadLocalValue(const DependencyProperty& property) const;
        PropertyValueSource GetValueSource(const DependencyProperty& property) const;
        std::any GetRawValue(const DependencyProperty& property) const;

    protected:
        bool IsApplyingPropertyValue() const noexcept { return m_isApplyingPropertyValue; }

        void SetValueInternal(const DependencyProperty& property,
                              std::any value,
                              PropertyValueSource source);

        virtual void OnDependencyPropertyChanged(const DependencyProperty& property,
                                                 const PropertyMetadata& metadata,
                                                 const std::any& oldValue,
                                                 const std::any& newValue,
                                                 PropertyValueSource source) {}

    private:
        std::any GetRawValueNoLock(const DependencyProperty& property) const;
        friend class ::NeoWPF::UI::Data::BindingOperations;

        static std::optional<std::any> FindRawValue(const DependencyObject* obj,
                                                    const DependencyProperty& property);

        void SetBindingExpression(const DependencyProperty& property,
                                  std::shared_ptr<::NeoWPF::UI::Data::BindingExpression> expression);
        std::shared_ptr<::NeoWPF::UI::Data::BindingExpression> GetBindingExpression(const DependencyProperty& property) const;
        void ClearBindingExpression(const DependencyProperty& property);

        bool m_isApplyingPropertyValue = false;
        std::unordered_map<const DependencyProperty*, std::any> m_expressionValues;
        std::unordered_map<const DependencyProperty*, std::any> m_localValues;
        std::unordered_map<const DependencyProperty*, std::any> m_styleValues;
        std::unordered_map<const DependencyProperty*, std::shared_ptr<::NeoWPF::UI::Data::BindingExpression>> m_bindingExpressions;
        mutable std::mutex m_propertyMutex;
    };

}
