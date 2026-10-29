/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
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

    /*
        @summary
        Reports whether the culture contains a valid language tag.
    */
    bool IsValid() const noexcept { return !m_name.empty(); }
    /*
        @summary
        Returns the normalized culture name.
    */
    const std::wstring& GetName() const noexcept { return m_name; }
    /*
        @summary
        Returns the language component of the culture.
    */
    const std::wstring& GetLanguage() const noexcept { return m_language; }
    /*
        @summary
        Returns the region component of the culture.
    */
    const std::wstring& GetRegion() const noexcept { return m_region; }
    /*
        @summary
        Returns the culture fallback chain.

        @returns
        The culture followed by its language-only fallback when applicable.
    */
    std::vector<Culture> GetFallbackChain() const;

    /*
        @summary
        Compares cultures by their normalized names.

        @param left
        First culture.

        @param right
        Second culture.
    */
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
