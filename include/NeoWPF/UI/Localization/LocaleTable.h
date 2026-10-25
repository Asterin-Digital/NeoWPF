/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Localization/Culture.h>
#include <NeoWPF/UI/Localization/LocaleString.h>
#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace NeoWPF::UI::Localization {

/*
    @summary
    Stores localized strings for one culture.
*/
class LocaleTable : public Properties::DependencyObject {
public:
    LocaleTable() = default;

    /*
        @summary
        Creates a locale table for a culture.

        @param culture
        Culture represented by the table.
    */
    explicit LocaleTable(const Culture& culture)
        : m_culture(culture)
    {
    }

    LocaleTable(const LocaleTable& other)
        : m_culture(other.m_culture)
        , m_strings(other.m_strings)
    {
    }

    LocaleTable& operator=(const LocaleTable& other)
    {
        m_culture = other.m_culture;
        m_strings = other.m_strings;
        return *this;
    }

    static const Properties::DependencyProperty& CultureProperty();
    static const Properties::DependencyProperty& StringsProperty();

    /*
        @summary
        Sets the culture represented by the table.

        @param culture
        Culture name.
    */
    void SetCulture(const std::wstring& culture);

    /*
        @summary
        Returns the locale strings contained in this table.
    */
    const std::vector<std::shared_ptr<LocaleString>>& GetLocaleStrings() const noexcept { return m_localeStrings; }

    void AddLocaleString(std::shared_ptr<LocaleString> value);
    
    explicit LocaleTable(const std::wstring& culture)
        : m_culture(culture)
    {
    }

    bool IsValid() const noexcept { return m_culture.IsValid(); }

    /*
        @summary
        Returns the culture represented by this locale table.
    */
    const Culture& GetCulture() const noexcept { return m_culture; }

    /*
        @summary
        Adds a locale string to the table.

        @param value
        Locale string to add.

        @returns
        True when the key was added.
    */
    bool Add(const LocaleString& value);

    /*
        @summary
        Adds a key and value to the table.

        @param key
        Localization key.

        @param value
        Localized text.

        @returns
        True when the key was added.
    */
    bool Add(const std::wstring& key, const std::wstring& value);

    /*
        @summary
        Looks up a localized value.

        @param key
        Localization key.

        @param value
        Receives the localized text.

        @returns
        True when the key exists.
    */
    bool TryGet(const std::wstring& key, std::wstring& value) const;
    const std::unordered_map<std::wstring, LocaleString>& GetStrings() const noexcept { return m_strings; }

private:
    Culture m_culture;
    std::unordered_map<std::wstring, LocaleString> m_strings;
    std::vector<std::shared_ptr<LocaleString>> m_localeStrings;
};

}
