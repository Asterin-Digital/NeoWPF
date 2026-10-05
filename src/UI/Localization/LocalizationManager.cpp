#include <NeoWPF/UI/Localization/LocalizationManager.h>

namespace NeoWPF::UI::Localization {

LocalizationManager& LocalizationManager::Get()
{
    static LocalizationManager instance;
    return instance;
}

bool LocalizationManager::SetTable(const LocaleTable& table, bool replace)
{
    if (!table.IsValid()) {
        return false;
    }

    {
        std::unique_lock lock(m_mutex);
        const auto key = table.GetCulture().GetName();
        const auto iterator = m_tables.find(key);
        if (iterator != m_tables.end() && !replace) {
            return false;
        }
        m_tables.insert_or_assign(key, table);
    }

    NotifySubscribers();
    return true;
}

bool LocalizationManager::RegisterTable(const LocaleTable& table)
{
    return SetTable(table, false);
}

bool LocalizationManager::ReplaceTable(const LocaleTable& table)
{
    return SetTable(table, true);
}

bool LocalizationManager::RemoveTable(const Culture& culture)
{
    if (!culture.IsValid()) {
        return false;
    }

    bool removed = false;
    {
        std::unique_lock lock(m_mutex);
        removed = m_tables.erase(culture.GetName()) != 0;
    }
    if (removed) {
        NotifySubscribers();
    }
    return removed;
}

void LocalizationManager::Clear()
{
    {
        std::unique_lock lock(m_mutex);
        m_tables.clear();
        m_activeLocale = Culture();
        m_fallbackLocale = Culture();
        m_missingKeyPolicy = MissingKeyPolicy::Marker;
    }
    NotifySubscribers();
}

bool LocalizationManager::SetActiveLocale(const Culture& culture)
{
    if (!culture.IsValid()) {
        return false;
    }

    {
        std::unique_lock lock(m_mutex);
        if (m_activeLocale == culture) {
            return true;
        }
        m_activeLocale = culture;
    }

    NotifySubscribers();
    return true;
}

bool LocalizationManager::SetActiveLocale(const std::wstring& culture)
{
    return SetActiveLocale(Culture(culture));
}

Culture LocalizationManager::GetActiveLocale() const
{
    std::shared_lock lock(m_mutex);
    return m_activeLocale;
}

bool LocalizationManager::SetFallbackLocale(const Culture& culture)
{
    if (!culture.IsValid()) {
        return false;
    }

    {
        std::unique_lock lock(m_mutex);
        if (m_fallbackLocale == culture) {
            return true;
        }
        m_fallbackLocale = culture;
    }

    NotifySubscribers();
    return true;
}

bool LocalizationManager::SetFallbackLocale(const std::wstring& culture)
{
    return SetFallbackLocale(Culture(culture));
}

void LocalizationManager::ClearFallbackLocale()
{
    {
        std::unique_lock lock(m_mutex);
        if (!m_fallbackLocale.IsValid()) {
            return;
        }
        m_fallbackLocale = Culture();
    }

    NotifySubscribers();
}

Culture LocalizationManager::GetFallbackLocale() const
{
    std::shared_lock lock(m_mutex);
    return m_fallbackLocale;
}

void LocalizationManager::SetMissingKeyPolicy(MissingKeyPolicy policy)
{
    {
        std::unique_lock lock(m_mutex);
        if (m_missingKeyPolicy == policy) {
            return;
        }
        m_missingKeyPolicy = policy;
    }

    NotifySubscribers();
}

MissingKeyPolicy LocalizationManager::GetMissingKeyPolicy() const
{
    std::shared_lock lock(m_mutex);
    return m_missingKeyPolicy;
}

bool LocalizationManager::TryResolveLocked(const std::wstring& key, std::wstring& value) const
{
    if (key.empty()) {
        return false;
    }

    const auto tryCulture = [this, &key, &value](const Culture& culture) {
        for (const auto& candidate : culture.GetFallbackChain()) {
            const auto table = m_tables.find(candidate.GetName());
            if (table != m_tables.end() && table->second.TryGet(key, value)) {
                return true;
            }
        }
        return false;
    };

    return tryCulture(m_activeLocale) || tryCulture(m_fallbackLocale);
}

bool LocalizationManager::TryResolve(const std::wstring& key, std::wstring& value) const
{
    std::shared_lock lock(m_mutex);
    return TryResolveLocked(key, value);
}

std::wstring LocalizationManager::Resolve(const std::wstring& key) const
{
    std::shared_lock lock(m_mutex);
    std::wstring value;
    if (TryResolveLocked(key, value)) {
        return value;
    }

    switch (m_missingKeyPolicy) {
    case MissingKeyPolicy::Empty:
        return {};
    case MissingKeyPolicy::Key:
        return key;
    case MissingKeyPolicy::Marker:
    default:
        return L"[[" + key + L"]]";
    }
}

void LocalizationManager::NotifyLocaleChanged()
{
    NotifySubscribers();
}

std::size_t LocalizationManager::Subscribe(std::function<void()> handler)
{
    return m_notifications.Subscribe(std::move(handler));
}

void LocalizationManager::Unsubscribe(std::size_t subscriptionId)
{
    m_notifications.Unsubscribe(subscriptionId);
}

void LocalizationManager::NotifySubscribers() const
{
    m_notifications.Publish();
}

}
