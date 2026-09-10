#pragma once

#include <string>
#include <vector>

namespace NeoWPF::UI::Localization {

class Culture {
public:
    Culture() = default;
    explicit Culture(const std::wstring& name);

    bool IsValid() const noexcept { return !m_name.empty(); }
    const std::wstring& GetName() const noexcept { return m_name; }
    const std::wstring& GetLanguage() const noexcept { return m_language; }
    const std::wstring& GetRegion() const noexcept { return m_region; }
    std::vector<Culture> GetFallbackChain() const;

    friend bool operator==(const Culture& left, const Culture& right) noexcept
    {
        return left.m_name == right.m_name;
    }

private:
    std::wstring m_name;
    std::wstring m_language;
    std::wstring m_region;
};

} // namespace NeoWPF::UI::Localization
