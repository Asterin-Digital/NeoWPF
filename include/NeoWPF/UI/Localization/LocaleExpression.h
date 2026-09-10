#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <atomic>
#include <memory>
#include <string>

namespace NeoWPF::UI::Localization {

class LocaleExpression {
public:
    LocaleExpression(Properties::DependencyObject& target,
                     const Properties::DependencyProperty& property,
                     std::wstring key);
    ~LocaleExpression();

    void Attach();
    void Detach();
    void Refresh();

    const std::wstring& GetKey() const noexcept { return m_key; }
    const Properties::DependencyProperty& GetTargetProperty() const noexcept { return m_targetProperty; }
    Properties::DependencyObject& GetTarget() const noexcept { return m_target; }

private:
    struct LifetimeState {
        std::atomic<bool> active{true};
    };

    Properties::DependencyObject& m_target;
    const Properties::DependencyProperty& m_targetProperty;
    std::wstring m_key;
    std::shared_ptr<LifetimeState> m_lifetimeState = std::make_shared<LifetimeState>();
    std::size_t m_subscriptionId = 0;
    bool m_isAttached = false;
};

} // namespace NeoWPF::UI::Localization