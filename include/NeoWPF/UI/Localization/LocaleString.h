#pragma once

#include <string>
#include <utility>

namespace NeoWPF::UI::Localization {

class LocaleString {
public:
    LocaleString() = default;
    LocaleString(std::wstring key, std::wstring value)
        : m_key(std::move(key))
        , m_value(std::move(value))
    {
    }

    const std::wstring& GetKey() const noexcept { return m_key; }
    const std::wstring& GetValue() const noexcept { return m_value; }

private:
    std::wstring m_key;
    std::wstring m_value;
};

}
