#include <NeoWPF/UI/Localization/Culture.h>
#include <NeoWPF/Platform/PlatformUtils.h>

#include <algorithm>

namespace NeoWPF::UI::Localization {
namespace {


}

Culture::Culture(const std::wstring& name)
{
    if (name.empty()) {
        return;
    }

    std::wstring normalized = name;
    std::replace(normalized.begin(), normalized.end(), L'_', L'-');

    const auto separator = normalized.find(L'-');
    const auto language = normalized.substr(0, separator);
    const auto region = separator == std::wstring::npos
        ? std::wstring()
        : normalized.substr(separator + 1);

    if (language.size() < 2 || language.size() > 8 ||
        !std::all_of(language.begin(), language.end(), NeoWPF::Platform::IsAsciiAlpha)) {
        return;
    }

    if (separator != std::wstring::npos && region.empty()) {
        return;
    }

    if (!region.empty() &&
        ((region.size() != 2 && region.size() != 3) ||
         !std::all_of(region.begin(), region.end(), [](wchar_t character) {
             return NeoWPF::Platform::IsAsciiAlpha(character) ||
                    NeoWPF::Platform::IsAsciiDigit(character);
         }))) {
        return;
    }

    m_language = language;
    m_language = NeoWPF::Platform::ToLower(language);
    m_region = region;
    m_region = NeoWPF::Platform::ToUpper(m_region);
    m_name = m_language;
    if (!m_region.empty()) {
        m_name += L'-';
        m_name += m_region;
    }
}

std::vector<Culture> Culture::GetFallbackChain() const
{
    if (!IsValid()) {
        return {};
    }

    std::vector<Culture> chain;
    chain.push_back(*this);
    if (!m_region.empty()) {
        chain.emplace_back(m_language);
    }
    return chain;
}

} // namespace NeoWPF::UI::Localization
