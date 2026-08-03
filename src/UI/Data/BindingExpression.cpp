#include <NeoWPF/UI/Data/BindingExpression.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI::Data {

BindingExpression::BindingExpression(Properties::DependencyObject& target,
                                     const Properties::DependencyProperty& property,
                                     const Binding& binding)
    : m_target(target)
    , m_targetProperty(property)
    , m_binding(binding)
{
}

BindingExpression::~BindingExpression()
{
    Detach();
}

void BindingExpression::Attach()
{
    if (Binding::s_dataBindingDebugging) {
        NeoWPF::Diagnostics::LogInfo("[Binding] Attach target=%p path=%s", &m_target, m_binding.GetPath().c_str());
    }

    Detach();

    const auto sourceLock = m_binding.GetSourceObject();
    if (sourceLock.has_value()) {
        if (auto source = sourceLock->lock()) {
            m_source = source;
            if (Binding::s_dataBindingDebugging) {
                NeoWPF::Diagnostics::LogInfo("[Binding] Explicit Source object resolved=%p", source.get());
            }
        }
    }

    if (!m_source) {
        const auto* dataContextProperty = Properties::DependencyPropertyRegistry::Get().FindProperty("DataContext", m_target);
        if (dataContextProperty) {
            const auto dataContextValue = m_target.GetRawValue(*dataContextProperty);
            if (dataContextValue.has_value()) {
                if (auto source = std::any_cast<std::shared_ptr<Properties::DependencyObject>>(dataContextValue)) {
                    m_source = source;
                } else if (auto sourcePoco = std::any_cast<std::shared_ptr<INotifyPropertyChanged>>(dataContextValue)) {
                    m_sourcePoco = sourcePoco;
                }
            }
        }
    }

    if (!m_source && !m_sourcePoco) {
        if (Binding::s_dataBindingDebugging) {
            NeoWPF::Diagnostics::LogInfo("[Binding] No source found for path '%s'", m_binding.GetPath().c_str());
        }
        return;
    }

    if (m_source) {
        m_sourceProperty = Properties::DependencyPropertyRegistry::Get().FindProperty(m_binding.GetPath(), *m_source);
        if (!m_sourceProperty) {
            if (Binding::s_dataBindingDebugging) {
                NeoWPF::Diagnostics::LogInfo("[Binding] Source property '%s' not found on source %p", m_binding.GetPath().c_str(), m_source.get());
            }
            return;
        }

        if (Binding::s_dataBindingDebugging) {
            NeoWPF::Diagnostics::LogInfo("[Binding] Subscribing to source property '%s' on %p", m_binding.GetPath().c_str(), m_source.get());
        }

        m_sourceChangeHandlerId = m_source->AddDependencyPropertyChangedHandler(*m_sourceProperty,
            [this](const std::any&, const std::any& newValue) {
                if (!m_target.GetBindingExpression(m_targetProperty)) {
                    return;
                }
                if (Binding::s_dataBindingDebugging) {
                    NeoWPF::Diagnostics::LogInfo("[Binding] Source property changed '%s' -> %s", m_binding.GetPath().c_str(), newValue.has_value() ? newValue.type().name() : "<empty>");
                }
                m_target.SetValueExpression(m_targetProperty, newValue);
            });

        const auto currentValue = m_source->GetRawValue(*m_sourceProperty);
        if (Binding::s_dataBindingDebugging) {
            NeoWPF::Diagnostics::LogInfo("[Binding] Initial source value for '%s' is %s", m_binding.GetPath().c_str(), currentValue.has_value() ? currentValue.type().name() : "<empty>");
        }
        m_target.SetValueExpression(m_targetProperty, currentValue);
        return;
    }

    if (m_sourcePoco) {
        if (Binding::s_dataBindingDebugging) {
            NeoWPF::Diagnostics::LogInfo("[Binding] Subscribing to INotifyPropertyChanged source for '%s'", m_binding.GetPath().c_str());
        }
        m_sourcePocoHandlerId = m_sourcePoco->AddPropertyChangedHandler([this](const std::string& propertyName) {
            if (propertyName != m_binding.GetPath()) {
                return;
            }
            if (!m_target.GetBindingExpression(m_targetProperty)) {
                return;
            }
            if (Binding::s_dataBindingDebugging) {
                NeoWPF::Diagnostics::LogInfo("[Binding] POCO property changed '%s'", propertyName.c_str());
            }
            const auto currentValue = m_sourcePoco->GetPropertyValue(propertyName);
            if (currentValue.has_value()) {
                m_target.SetValueExpression(m_targetProperty, *currentValue);
            }
        });

        const auto currentValue = m_sourcePoco->GetPropertyValue(m_binding.GetPath());
        if (Binding::s_dataBindingDebugging) {
            NeoWPF::Diagnostics::LogInfo("[Binding] Initial POCO source value for '%s' is %s", m_binding.GetPath().c_str(), currentValue.has_value() ? currentValue->type().name() : "<empty>");
        }
        if (currentValue.has_value()) {
            m_target.SetValueExpression(m_targetProperty, *currentValue);
        }
    }
}

void BindingExpression::Detach()
{
    if (Binding::s_dataBindingDebugging) {
        NeoWPF::Diagnostics::LogInfo("[Binding] Detach target=%p path=%s", &m_target, m_binding.GetPath().c_str());
    }
    if (m_source && m_sourceProperty && m_sourceChangeHandlerId != 0) {
        m_source->RemoveDependencyPropertyChangedHandler(*m_sourceProperty, m_sourceChangeHandlerId);
        m_sourceChangeHandlerId = 0;
    }
    if (m_sourcePoco && m_sourcePocoHandlerId != 0) {
        m_sourcePoco->RemovePropertyChangedHandler(m_sourcePocoHandlerId);
        m_sourcePocoHandlerId = 0;
    }
    m_source.reset();
    m_sourcePoco.reset();
    m_sourceProperty = nullptr;
}

void BindingExpression::Refresh()
{
    if (GetTarget().GetBindingExpression(GetTargetProperty()).get() != this) {
        return;
    }

    if (Binding::s_dataBindingDebugging) {
        NeoWPF::Diagnostics::LogInfo("[Binding] Refresh target=%p path=%s", &m_target, m_binding.GetPath().c_str());
    }
    Attach();
}

}
