/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <string>
#include <any>
#include <utility>
#include <vector>

#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Data/Binding.h>

namespace NeoWPF::UI::Data {
class INotifyPropertyChanged;
}

namespace NeoWPF::UI::Data {

/*
    @summary
    Applies a binding to a dependency property.
*/
class BindingExpression {
public:
    /*
        @summary
        Creates a binding expression.

        @param target
        Object whose property is updated.

        @param property
        Property to update.

        @param binding
        Binding configuration.
    */
    BindingExpression(Properties::DependencyObject& target,
                      const Properties::DependencyProperty& property,
                      const Binding& binding);

    ~BindingExpression();

    /*
        @summary
        Starts applying the binding.
    */
    void Attach();

    /*
        @summary
        Stops applying the binding.
    */
    void Detach();

    /*
        @summary
        Applies the current source value to the target.
    */
    void Refresh();

    const Properties::DependencyProperty& GetTargetProperty() const noexcept {
        return m_targetProperty;
    }

    Properties::DependencyObject& GetTarget() const noexcept {
        return m_target;
    }

private:
    Properties::DependencyObject& m_target;
    const Properties::DependencyProperty& m_targetProperty;
    Binding m_binding;
    std::shared_ptr<Properties::DependencyObject> m_source;
    std::shared_ptr<INotifyPropertyChanged> m_sourcePoco;
    const Properties::DependencyProperty* m_sourceProperty = nullptr;
    std::size_t m_sourceChangeHandlerId = 0;
    std::size_t m_targetChangeHandlerId = 0;
    std::vector<std::pair<std::shared_ptr<INotifyPropertyChanged>, std::size_t>> m_sourcePocoHandlerIds;
    bool m_isApplyingTargetValue = false;
    bool m_isApplyingSourceValue = false;
    bool m_isDetaching = false;

    void UpdatePocoTargetValue();
    void SubscribePocoPath();
    void UnsubscribePocoHandlers();
    bool TrySetPocoSourceValue(const std::shared_ptr<INotifyPropertyChanged>& source,
                               const std::vector<std::string>& segments,
                               const std::any& value);
};

}
