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

class LocaleTable : public Properties::DependencyObject {
public:
    LocaleTable() = default;
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

    void SetCulture(const std::wstring& culture);
    const std::vector<std::shared_ptr<LocaleString>>& GetLocaleStrings() const noexcept { return m_localeStrings; }
    void AddLocaleString(std::shared_ptr<LocaleString> value);
    explicit LocaleTable(const std::wstring& culture)
        : m_culture(culture)
    {
    }

    bool IsValid() const noexcept { return m_culture.IsValid(); }
    const Culture& GetCulture() const noexcept { return m_culture; }

    bool Add(const LocaleString& value);
    bool Add(const std::wstring& key, const std::wstring& value);
    bool TryGet(const std::wstring& key, std::wstring& value) const;
    const std::unordered_map<std::wstring, LocaleString>& GetStrings() const noexcept { return m_strings; }

private:
    Culture m_culture;
    std::unordered_map<std::wstring, LocaleString> m_strings;
    std::vector<std::shared_ptr<LocaleString>> m_localeStrings;
};

}
