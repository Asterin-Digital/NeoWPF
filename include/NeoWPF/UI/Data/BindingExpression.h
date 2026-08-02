#pragma once

#include <memory>
#include <string>
#include <any>

#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Data/Binding.h>
#include <NeoWPF/UI/Data/INotifyPropertyChanged.h>

namespace NeoWPF::UI::Data {

class BindingExpression {
public:
    BindingExpression(Properties::DependencyObject& target,
                      const Properties::DependencyProperty& property,
                      const Binding& binding);

    ~BindingExpression();

    void Attach();
    void Detach();
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
    std::size_t m_sourcePocoHandlerId = 0;
};

}
