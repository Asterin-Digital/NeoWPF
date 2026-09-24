/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

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

    /*
        @summary
        Registers one application-owned translation table.
    */
    bool RegisterTable(const LocaleTable& table);

    /*
        @summary
        Replaces the translation table for a culture.
    */
    bool ReplaceTable(const LocaleTable& table);

    /*
        @summary
        Removes the translation table for a culture.
    */
    bool RemoveTable(const Culture& culture);

    /*
        @summary
        Removes all registered tables and resets locale configuration.
    */
    void Clear();

    /*
        @summary
        Sets the active locale and refreshes attached localization expressions.
    */
    bool SetActiveLocale(const Culture& culture);
    bool SetActiveLocale(const std::wstring& culture);

    /*
        @summary
        Returns the active locale.
    */
    Culture GetActiveLocale() const;

    /*
        @summary
        Sets the locale used when the active locale has no matching translation.
    */
    bool SetFallbackLocale(const Culture& culture);
    bool SetFallbackLocale(const std::wstring& culture);

    /*
        @summary
        Removes the configured fallback locale.
    */
    void ClearFallbackLocale();

    /*
        @summary
        Returns the configured fallback locale.
    */
    Culture GetFallbackLocale() const;

    /*
        @summary
        Sets the behavior used when a key cannot be resolved.
    */
    void SetMissingKeyPolicy(MissingKeyPolicy policy);

    /*
        @summary
        Returns the current missing-key policy.
    */
    MissingKeyPolicy GetMissingKeyPolicy() const;

    /*
        @summary
        Attempts to resolve a localized string.

        @param key
        The application-owned localization key.

        @param value
        Receives the resolved string when the key is found.

        @returns
        `true` when a translation was found.
    */
    bool TryResolve(const std::wstring& key, std::wstring& value) const;

    /*
        @summary
        Resolves a localized string using the active locale and fallback rules.
    */
    std::wstring Resolve(const std::wstring& key) const;

    void NotifyLocaleChanged();

    /*
        @summary
        Subscribes to localization changes.

        @param handler
        Callback invoked after localization state changes.

        @returns
        The subscription identifier, or zero for an invalid handler.
    */
    std::size_t Subscribe(std::function<void()> handler);

    /*
        @summary
        Removes a localization-change subscription.
    */
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
