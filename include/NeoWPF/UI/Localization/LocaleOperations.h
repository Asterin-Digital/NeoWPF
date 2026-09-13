/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <memory>
#include <string>

namespace NeoWPF::UI::Localization {

class LocaleExpression;

/*
    @summary
    Provides operations for assigning localized values to dependency properties.
*/
class LocaleOperations {
public:
    /*
        @summary
        Assigns a localization key to a dependency property.

        @param target
        Object whose property is updated.

        @param property
        Property to update.

        @param key
        Localization key.

        @returns
        The attached expression, or null for an empty key.
    */
    static std::shared_ptr<LocaleExpression> SetLocale(
        Properties::DependencyObject& target,
        const Properties::DependencyProperty& property,
        const std::wstring& key);

    static std::shared_ptr<LocaleExpression> SetLocale(
        Properties::DependencyObject& target,
        const Properties::DependencyProperty* property,
        const std::wstring& key);

    /*
        @summary
        Removes a localization expression from a dependency property.

        @param target
        Object whose property is cleared.

        @param property
        Property to clear.
    */
    static void ClearLocale(Properties::DependencyObject& target,
                             const Properties::DependencyProperty& property);
    static void ClearLocale(Properties::DependencyObject& target,
                            const Properties::DependencyProperty* property);

    /*
        @summary
        Returns the localization expression assigned to a dependency property.

        @param target
        Object to inspect.

        @param property
        Property to inspect.
    */
    static std::shared_ptr<LocaleExpression> GetLocaleExpression(
        const Properties::DependencyObject& target,
        const Properties::DependencyProperty& property);
    static std::shared_ptr<LocaleExpression> GetLocaleExpression(
        const Properties::DependencyObject& target,
        const Properties::DependencyProperty* property);
};

}