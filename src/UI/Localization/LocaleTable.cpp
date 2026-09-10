#include <NeoWPF/UI/Localization/LocaleTable.h>

namespace NeoWPF::UI::Localization {

bool LocaleTable::Add(const LocaleString& value)
{
    if (!IsValid() || value.GetKey().empty()) {
        return false;
    }

    return m_strings.emplace(value.GetKey(), value).second;
}

bool LocaleTable::Add(const std::wstring& key, const std::wstring& value)
{
    return Add(LocaleString(key, value));
}

bool LocaleTable::TryGet(const std::wstring& key, std::wstring& value) const
{
    const auto iterator = m_strings.find(key);
    if (iterator == m_strings.end()) {
        return false;
    }

    value = iterator->second.GetValue();
    return true;
}

}
