#pragma once

#include <NeoWPF/UI/Localization/Culture.h>
#include <NeoWPF/UI/Localization/LocaleString.h>

#include <string>
#include <unordered_map>

namespace NeoWPF::UI::Localization {

class LocaleTable {
public:
    LocaleTable() = default;
    explicit LocaleTable(const Culture& culture)
        : m_culture(culture)
    {
    }
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
};

}
