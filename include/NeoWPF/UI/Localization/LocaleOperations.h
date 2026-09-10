#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <memory>
#include <string>

namespace NeoWPF::UI::Localization {

class LocaleExpression;

class LocaleOperations {
public:
    static std::shared_ptr<LocaleExpression> SetLocale(
        Properties::DependencyObject& target,
        const Properties::DependencyProperty& property,
        const std::wstring& key);

    static std::shared_ptr<LocaleExpression> SetLocale(
        Properties::DependencyObject& target,
        const Properties::DependencyProperty* property,
        const std::wstring& key);

    static void ClearLocale(Properties::DependencyObject& target,
                             const Properties::DependencyProperty& property);
    static void ClearLocale(Properties::DependencyObject& target,
                            const Properties::DependencyProperty* property);

    static std::shared_ptr<LocaleExpression> GetLocaleExpression(
        const Properties::DependencyObject& target,
        const Properties::DependencyProperty& property);
    static std::shared_ptr<LocaleExpression> GetLocaleExpression(
        const Properties::DependencyObject& target,
        const Properties::DependencyProperty* property);
};

} // namespace NeoWPF::UI::Localization