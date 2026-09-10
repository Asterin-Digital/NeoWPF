#include <NeoWPF/UI/Localization/LocaleExpression.h>

#include <NeoWPF/UI/Localization/LocalizationManager.h>

namespace NeoWPF::UI::Localization {

LocaleExpression::LocaleExpression(Properties::DependencyObject& target,
                                   const Properties::DependencyProperty& property,
                                   std::wstring key)
    : m_target(target)
    , m_targetProperty(property)
    , m_key(std::move(key))
{
}

LocaleExpression::~LocaleExpression()
{
    Detach();
    m_lifetimeState->active.store(false);
}

void LocaleExpression::Attach()
{
    if (m_isAttached) {
        Refresh();
        return;
    }

    if (m_key.empty()) {
        return;
    }

    const auto lifetimeState = m_lifetimeState;
    m_subscriptionId = LocalizationManager::Get().Subscribe([this, lifetimeState] {
        if (lifetimeState->active.load()) {
            Refresh();
        }
    });
    m_isAttached = m_subscriptionId != 0;
    if (m_isAttached) {
        Refresh();
    }
}

void LocaleExpression::Detach()
{
    if (!m_isAttached) {
        return;
    }

    m_lifetimeState->active.store(false);
    LocalizationManager::Get().Unsubscribe(m_subscriptionId);
    m_subscriptionId = 0;
    m_isAttached = false;
}

void LocaleExpression::Refresh()
{
    if (!m_isAttached || m_key.empty()) {
        return;
    }

    m_target.SetValueExpression(m_targetProperty,
                                LocalizationManager::Get().Resolve(m_key));
}

} // namespace NeoWPF::UI::Localization