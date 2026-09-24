/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <string>
#include <vector>

namespace NeoWPF::UI::Localization {

/*
    @summary
    Represents a normalized language and region.
*/
class Culture {
public:
    Culture() = default;

    /*
        @summary
        Creates a culture from a language tag.

        @param name
        Language tag such as en or en-US.
    */
    explicit Culture(const std::wstring& name);

    bool IsValid() const noexcept { return !m_name.empty(); }
    const std::wstring& GetName() const noexcept { return m_name; }
    const std::wstring& GetLanguage() const noexcept { return m_language; }
    const std::wstring& GetRegion() const noexcept { return m_region; }
    /*
        @summary
        Returns the culture fallback chain.

        @returns
        The culture followed by its language-only fallback when applicable.
    */
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

}
