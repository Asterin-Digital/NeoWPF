/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Properties/DependencyObject.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

#include <string>
#include <utility>

namespace NeoWPF::UI::Localization {

/*
    @summary
    Stores one localized key and value pair.
*/
class LocaleString : public Properties::DependencyObject {
public:
    using Properties::DependencyObject::SetValue;

    LocaleString() = default;

    /*
        @summary
        Creates a locale string.

        @param key
        Localization key.

        @param value
        Localized text.
    */
    LocaleString(std::wstring key, std::wstring value)
        : m_key(std::move(key))
        , m_value(std::move(value))
    {
    }

    LocaleString(const LocaleString& other)
        : m_key(other.m_key)
        , m_value(other.m_value)
    {
    }

    LocaleString& operator=(const LocaleString& other)
    {
        m_key = other.m_key;
        m_value = other.m_value;
        return *this;
    }

    static const Properties::DependencyProperty& KeyProperty();
    static const Properties::DependencyProperty& ValueProperty();

    /*
        @summary
        Sets the localization key.

        @param key
        Localization key.
    */
    void SetKey(const std::wstring& key);

    /*
        @summary
        Sets the localized text.

        @param value
        Localized text.
    */
    void SetValue(const std::wstring& value);

    /*
        @summary
        Returns the localization key.
    */
    const std::wstring& GetKey() const noexcept { return m_key; }

    /*
        @summary
        Returns the localized text value.
    */
    const std::wstring& GetValue() const noexcept { return m_value; }

private:
    std::wstring m_key;
    std::wstring m_value;
};

}
