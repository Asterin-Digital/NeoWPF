#pragma once

#include <NeoWPF/UI/Localization/Culture.h>
#include <NeoWPF/UI/Localization/LocaleTable.h>

#include <mutex>
#include <shared_mutex>
#include <string>
#include <functional>
#include <unordered_map>

namespace NeoWPF::UI::Localization {

enum class MissingKeyPolicy {
    Marker,
    Empty,
    Key
};

class LocalizationManager {
public:
    static LocalizationManager& Get();
    static LocalizationManager& Instance() { return Get(); }

    bool RegisterTable(const LocaleTable& table);
    bool ReplaceTable(const LocaleTable& table);
    bool RemoveTable(const Culture& culture);
    void Clear();

    bool SetActiveLocale(const Culture& culture);
    bool SetActiveLocale(const std::wstring& culture);
    Culture GetActiveLocale() const;

    bool SetFallbackLocale(const Culture& culture);
    bool SetFallbackLocale(const std::wstring& culture);
    void ClearFallbackLocale();
    Culture GetFallbackLocale() const;

    void SetMissingKeyPolicy(MissingKeyPolicy policy);
    MissingKeyPolicy GetMissingKeyPolicy() const;

    bool TryResolve(const std::wstring& key, std::wstring& value) const;
    std::wstring Resolve(const std::wstring& key) const;

    std::size_t Subscribe(std::function<void()> handler);
    void Unsubscribe(std::size_t subscriptionId);

private:
    LocalizationManager() = default;

    bool SetTable(const LocaleTable& table, bool replace);
    bool TryResolveLocked(const std::wstring& key, std::wstring& value) const;
    void NotifySubscribers() const;

    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::wstring, LocaleTable> m_tables;
    Culture m_activeLocale;
    Culture m_fallbackLocale;
    MissingKeyPolicy m_missingKeyPolicy = MissingKeyPolicy::Marker;
    std::unordered_map<std::size_t, std::function<void()>> m_subscribers;
    std::size_t m_nextSubscriptionId = 1;
};

}
