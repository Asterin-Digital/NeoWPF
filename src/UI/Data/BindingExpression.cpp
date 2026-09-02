#include <NeoWPF/UI/Data/BindingExpression.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/UI/Data/INotifyPropertyChanged.h>
#include <NeoWPF/UI/Properties/Converters/CommonConverters.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Diagnostics/DeveloperWarnings.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <typeinfo>
#include <vector>

namespace NeoWPF::UI::Data {
namespace {

std::vector<std::string> SplitPropertyPath(const std::string& path)
{
    std::vector<std::string> segments;
    std::string current;
    for (const char ch : path) {
        if (ch == '.') {
            if (!current.empty()) {
                segments.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(ch);
        }
    }
    if (!current.empty()) {
        segments.push_back(current);
    }
    return segments;
}

std::optional<std::any> ResolvePocoValue(const std::shared_ptr<INotifyPropertyChanged>& source,
                                         const std::string& path)
{
    if (!source || path.empty()) {
        return std::nullopt;
    }

    std::shared_ptr<INotifyPropertyChanged> currentSource = source;
    std::optional<std::any> currentValue;
    const auto segments = SplitPropertyPath(path);

    for (std::size_t index = 0; index < segments.size(); ++index) {
        if (!currentSource) {
            return std::nullopt;
        }

        currentValue = currentSource->GetPropertyValue(segments[index]);
        if (!currentValue.has_value()) {
            return std::nullopt;
        }

        if (index + 1 < segments.size()) {
            try {
                currentSource = std::any_cast<std::shared_ptr<INotifyPropertyChanged>>(*currentValue);
            } catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
    }

    return currentValue;
}

bool TrySetPocoValue(const std::shared_ptr<INotifyPropertyChanged>& source,
                     const std::vector<std::string>& segments,
                     const std::any& value)
{
    if (!source || segments.empty()) {
        return false;
    }

    std::shared_ptr<INotifyPropertyChanged> currentSource = source;
    for (std::size_t index = 0; index + 1 < segments.size(); ++index) {
        if (!currentSource) {
            return false;
        }

        const auto childValue = currentSource->GetPropertyValue(segments[index]);
        if (!childValue.has_value()) {
            return false;
        }

        try {
            currentSource = std::any_cast<std::shared_ptr<INotifyPropertyChanged>>(*childValue);
        } catch (const std::bad_any_cast&) {
            return false;
        }
    }

    if (!currentSource) {
        return false;
    }

    return currentSource->SetPropertyValue(segments.back(), value);
}

} // namespace

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

void BindingExpression::UpdatePocoTargetValue()
{
    const auto currentValue = ResolvePocoValue(m_sourcePoco, m_binding.GetPath());
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Updating POCO target for '%s' -> %s", m_binding.GetPath().c_str(), currentValue.has_value() ? currentValue->type().name() : "<empty>");
    if (currentValue.has_value()) {
        const bool previousApplying = m_isApplyingTargetValue;
        m_isApplyingTargetValue = true;
        std::any convertedValue;
        if (Properties::TryConvertValue(m_targetProperty, *currentValue, convertedValue)) {
            m_target.SetValueExpression(m_targetProperty, convertedValue);
        }
        m_isApplyingTargetValue = previousApplying;
    }
}

void BindingExpression::SubscribePocoPath()
{
    if (!m_sourcePoco) {
        return;
    }

    UnsubscribePocoHandlers();

    std::shared_ptr<INotifyPropertyChanged> currentSource = m_sourcePoco;
    const auto segments = SplitPropertyPath(m_binding.GetPath());

    for (std::size_t index = 0; index < segments.size(); ++index) {
        if (!currentSource) {
            break;
        }

        const auto handlerId = currentSource->AddPropertyChangedHandler([this](const std::string& propertyName) {
            (void)propertyName;
            if (!m_target.GetBindingExpression(m_targetProperty)) {
                return;
            }
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] POCO property changed '%s'", propertyName.c_str());
            UpdatePocoTargetValue();
        });
        m_sourcePocoHandlerIds.emplace_back(currentSource, handlerId);

        if (index + 1 >= segments.size()) {
            break;
        }

        const auto childValue = currentSource->GetPropertyValue(segments[index]);
        if (!childValue.has_value()) {
            break;
        }

        try {
            currentSource = std::any_cast<std::shared_ptr<INotifyPropertyChanged>>(*childValue);
        } catch (const std::bad_any_cast&) {
            break;
        }
    }
}

void BindingExpression::UnsubscribePocoHandlers()
{
    for (const auto& [source, handlerId] : m_sourcePocoHandlerIds) {
        if (source && handlerId != 0) {
            source->RemovePropertyChangedHandler(handlerId);
        }
    }
    m_sourcePocoHandlerIds.clear();
}

bool BindingExpression::TrySetPocoSourceValue(const std::shared_ptr<INotifyPropertyChanged>& source,
                                              const std::vector<std::string>& segments,
                                              const std::any& value)
{
    if (!source || segments.empty()) {
        return false;
    }

    std::shared_ptr<INotifyPropertyChanged> currentSource = source;
    for (std::size_t index = 0; index + 1 < segments.size(); ++index) {
        if (!currentSource) {
            return false;
        }

        const auto childValue = currentSource->GetPropertyValue(segments[index]);
        if (!childValue.has_value()) {
            return false;
        }

        try {
            currentSource = std::any_cast<std::shared_ptr<INotifyPropertyChanged>>(*childValue);
        } catch (const std::bad_any_cast&) {
            return false;
        }
    }

    if (!currentSource) {
        return false;
    }

    return currentSource->SetPropertyValue(segments.back(), value);
}

void BindingExpression::Attach()
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Attach target=%p path=%s", &m_target, m_binding.GetPath().c_str());

    Detach();

    const auto sourceLock = m_binding.GetSourceObject();
    if (sourceLock.has_value()) {
        if (auto source = sourceLock->lock()) {
            m_source = source;
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Explicit Source object resolved=%p", source.get());
        }
    }

    if (!m_source) {
        std::any dataContextValue;
        if (const auto* frameworkElement = dynamic_cast<const ::NeoWPF::UI::FrameworkElement*>(&m_target)) {
            dataContextValue = frameworkElement->GetDataContext();
        } else if (const auto* dataContextProperty = Properties::DependencyPropertyRegistry::Get().FindProperty("DataContext", m_target)) {
            dataContextValue = m_target.GetRawValue(*dataContextProperty);
        }

        if (dataContextValue.has_value()) {
            const auto& valueType = dataContextValue.type();
            if (valueType == typeid(std::shared_ptr<Properties::DependencyObject>)) {
                if (auto source = std::any_cast<std::shared_ptr<Properties::DependencyObject>>(dataContextValue)) {
                    m_source = source;
                    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] DataContext resolved as DependencyObject=%p", m_source.get());
                }
            } else if (valueType == typeid(std::shared_ptr<INotifyPropertyChanged>)) {
                if (auto sourcePoco = std::any_cast<std::shared_ptr<INotifyPropertyChanged>>(dataContextValue)) {
                    m_sourcePoco = sourcePoco;
                    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] DataContext resolved as INotifyPropertyChanged=%p", m_sourcePoco.get());
                }
            } else {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] DataContext has unsupported type=%s", valueType.name());
            }
        }
    }

    if (!m_source && !m_sourcePoco) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] No source found for path '%s'", m_binding.GetPath().c_str());
        return;
    }

    if (m_source) {
        m_sourceProperty = Properties::DependencyPropertyRegistry::Get().FindProperty(m_binding.GetPath(), *m_source);
        if (!m_sourceProperty) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Source property '%s' not found on source %p", m_binding.GetPath().c_str(), m_source.get());
            const std::wstring title = std::wstring(NeoWPF::Diagnostics::DeveloperWarnings::kInvalidDependencyPropertyCode) + L": invalid dependency property used in binding";
            NeoWPF::Diagnostics::QueueDeveloperWarning(title,
                std::wstring(L"Binding path '") + std::wstring(m_binding.GetPath().begin(), m_binding.GetPath().end()) + L"' could not be resolved on the source object. Verify the source property name and type.",
                NeoWPF::Diagnostics::DeveloperWarningSeverity::Warning);
            return;
        }

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Subscribing to source property '%s' on %p", m_binding.GetPath().c_str(), m_source.get());

        if (m_binding.GetMode() == BindingMode::TwoWay) {
            m_targetChangeHandlerId = m_target.AddDependencyPropertyChangedHandler(m_targetProperty,
                [this](const std::any&, const std::any& newValue) {
                    if (m_isApplyingTargetValue || m_isApplyingSourceValue ||
                        m_binding.GetMode() != BindingMode::TwoWay || !m_source || !m_sourceProperty) {
                        return;
                    }
                    if (!m_target.GetBindingExpression(m_targetProperty)) {
                        return;
                    }

                    const bool previousApplying = m_isApplyingSourceValue;
                    m_isApplyingSourceValue = true;
                    m_source->SetValue(*m_sourceProperty, newValue);
                    m_isApplyingSourceValue = previousApplying;
                });
        }

        m_sourceChangeHandlerId = m_source->AddDependencyPropertyChangedHandler(*m_sourceProperty,
            [this](const std::any&, const std::any& newValue) {
                if (!m_target.GetBindingExpression(m_targetProperty)) {
                    return;
                }
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Source property changed '%s' -> %s", m_binding.GetPath().c_str(), newValue.has_value() ? newValue.type().name() : "<empty>");
                const bool previousApplying = m_isApplyingTargetValue;
                m_isApplyingTargetValue = true;
                std::any convertedValue;
                if (Properties::TryConvertValue(m_targetProperty, newValue, convertedValue)) {
                    m_target.SetValueExpression(m_targetProperty, convertedValue);
                }
                m_isApplyingTargetValue = previousApplying;
            });

        const auto currentValue = m_source->GetRawValue(*m_sourceProperty);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Initial source value for '%s' is %s", m_binding.GetPath().c_str(), currentValue.has_value() ? currentValue.type().name() : "<empty>");
        std::any convertedValue;
        if (Properties::TryConvertValue(m_targetProperty, currentValue, convertedValue)) {
            m_target.SetValueExpression(m_targetProperty, convertedValue);
        }
        return;
    }

    if (m_sourcePoco) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Subscribing to INotifyPropertyChanged source for '%s'", m_binding.GetPath().c_str());

        if (m_binding.GetMode() == BindingMode::TwoWay) {
            if (m_targetChangeHandlerId == 0) {
                m_targetChangeHandlerId = m_target.AddDependencyPropertyChangedHandler(m_targetProperty,
                    [this](const std::any&, const std::any& newValue) {
                        if (m_isApplyingTargetValue || m_binding.GetMode() != BindingMode::TwoWay || !m_sourcePoco) {
                            return;
                        }
                        if (!m_target.GetBindingExpression(m_targetProperty)) {
                            return;
                        }
                                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Updating POCO source for '%s'", m_binding.GetPath().c_str());
                        const auto segments = SplitPropertyPath(m_binding.GetPath());
                        if (!TrySetPocoSourceValue(m_sourcePoco, segments, newValue)) {
                            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Failed to write POCO source for '%s'", m_binding.GetPath().c_str());
                        }
                    });
            }
        } else if (m_targetChangeHandlerId != 0) {
            m_target.RemoveDependencyPropertyChangedHandler(m_targetProperty, m_targetChangeHandlerId);
            m_targetChangeHandlerId = 0;
        }

        SubscribePocoPath();

        const auto currentValue = ResolvePocoValue(m_sourcePoco, m_binding.GetPath());
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Initial POCO source value for '%s' is %s", m_binding.GetPath().c_str(), currentValue.has_value() ? currentValue->type().name() : "<empty>");
        if (currentValue.has_value()) {
            const bool previousApplying = m_isApplyingTargetValue;
            m_isApplyingTargetValue = true;
            std::any convertedValue;
            if (Properties::TryConvertValue(m_targetProperty, *currentValue, convertedValue)) {
                m_target.SetValueExpression(m_targetProperty, convertedValue);
            }
            m_isApplyingTargetValue = previousApplying;
        }
    }
}

void BindingExpression::Detach()
{
    if (m_isDetaching) {
        return;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Detach target=%p path=%s", &m_target, m_binding.GetPath().c_str());
    m_isDetaching = true;

    if (m_source && m_sourceProperty && m_sourceChangeHandlerId != 0) {
        m_source->RemoveDependencyPropertyChangedHandler(*m_sourceProperty, m_sourceChangeHandlerId);
        m_sourceChangeHandlerId = 0;
    }
    if (m_targetChangeHandlerId != 0) {
        m_target.RemoveDependencyPropertyChangedHandler(m_targetProperty, m_targetChangeHandlerId);
        m_targetChangeHandlerId = 0;
    }
    UnsubscribePocoHandlers();
    m_source.reset();
    m_sourcePoco.reset();
    m_sourceProperty = nullptr;
    m_isDetaching = false;
}

void BindingExpression::Refresh()
{
    if (GetTarget().GetBindingExpression(GetTargetProperty()).get() != this) {
        return;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Application, NeoWPF::Diagnostics::Level::Info, "[Binding] Refresh target=%p path=%s", &m_target, m_binding.GetPath().c_str());
    Attach();
}

}
