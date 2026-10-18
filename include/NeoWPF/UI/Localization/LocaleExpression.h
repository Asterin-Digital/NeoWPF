/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace NeoWPF::UI::Localization {

/*
    @summary
    Binds a dependency property to a localized string.

    @details
    The property is refreshed when the active locale changes.
*/
class LocaleExpression {
public:
    /*
        @summary
        Creates a locale expression for a dependency property.

        @param target
        Object whose property is updated.

        @param property
        Property to update.

        @param key
        Localization key.
    */
    LocaleExpression(Properties::DependencyObject& target,
                     const Properties::DependencyProperty& property,
                     std::wstring key);
    ~LocaleExpression();

    /*
        @summary
        Starts updating the target property.
    */
    void Attach();

    /*
        @summary
        Stops updating the target property.
    */
    void Detach();

    /*
        @summary
        Applies the current localized value to the target property.
    */
    void Refresh();

    const std::wstring& GetKey() const noexcept { return m_key; }
    const Properties::DependencyProperty& GetTargetProperty() const noexcept { return m_targetProperty; }
    Properties::DependencyObject& GetTarget() const noexcept { return m_target; }

private:
    struct LifetimeState {
        std::recursive_mutex mutex;
        bool active = true;
    };

    Properties::DependencyObject& m_target;
    const Properties::DependencyProperty& m_targetProperty;
    std::wstring m_key;
    std::shared_ptr<LifetimeState> m_lifetimeState = std::make_shared<LifetimeState>();
    std::size_t m_subscriptionId = 0;
    std::atomic<bool> m_isAttached{false};
};

}