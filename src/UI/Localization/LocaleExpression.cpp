#include <NeoWPF/UI/Localization/LocaleExpression.h>

#include <NeoWPF/UI/Localization/LocalizationManager.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

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
    {
        std::lock_guard lock(lifetimeState->mutex);
        lifetimeState->active = true;
    }

    m_subscriptionId = LocalizationManager::Get().Subscribe([this, lifetimeState] {
        std::lock_guard lock(lifetimeState->mutex);
        if (!lifetimeState->active) {
            return;
        }
        Refresh();
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

    const auto lifetimeState = m_lifetimeState;
    {
        std::lock_guard lock(lifetimeState->mutex);
        lifetimeState->active = false;
    }

    LocalizationManager::Get().Unsubscribe(m_subscriptionId);
    m_subscriptionId = 0;
    m_isAttached = false;
}

void LocaleExpression::Refresh()
{
    if (!m_isAttached || m_key.empty()) {
        return;
    }

    std::wstring value;
    auto* element = dynamic_cast<NeoWPF::UI::UIElement*>(&m_target);
    while (element) {
        if (const auto* window = dynamic_cast<const NeoWPF::UI::Window*>(element);
            window && window->TryResolveLocale(m_key, value)) {
            m_target.SetValueExpression(m_targetProperty, value);
            return;
        }
        element = dynamic_cast<NeoWPF::UI::UIElement*>(element->GetParent());
    }

    if (!LocalizationManager::Get().TryResolve(m_key, value)) {
        value = LocalizationManager::Get().Resolve(m_key);
    }
    m_target.SetValueExpression(m_targetProperty, value);
}

}