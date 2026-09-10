#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Localization/LocalizationManager.h>

namespace NeoWPF::UI {

void Window::AddLocale(std::shared_ptr<NeoWPF::UI::Localization::LocaleTable> table)
{
    if (!table || !table->IsValid()) {
        return;
    }
    m_locales.push_back(std::move(table));
    NeoWPF::UI::Localization::LocalizationManager::Get().NotifyLocaleChanged();
}

bool Window::TryResolveLocale(const std::wstring& key, std::wstring& value) const
{
    const auto& manager = NeoWPF::UI::Localization::LocalizationManager::Get();
    const auto active = manager.GetActiveLocale();
    const auto tryCulture = [this, &key, &value](
        const NeoWPF::UI::Localization::Culture& culture) {
        for (const auto& candidate : culture.GetFallbackChain()) {
            for (const auto& table : m_locales) {
                if (table && table->GetCulture() == candidate && table->TryGet(key, value)) {
                    return true;
                }
            }
        }
        return false;
    };
    return tryCulture(active);
}

void Window::RebuildVisualTree() noexcept
{
    if (m_visualRoot) {
        RemoveVisualChild(m_visualRoot);
    }

    if (!m_content && !m_titleBar) {
        m_visualRoot = nullptr;
        return;
    }

    auto root = std::make_shared<Grid>();
    root->AddRow(SizeMode::Auto, 0);
    root->AddRow(SizeMode::Star, 1);
    root->AddColumn(SizeMode::Star, 1);

    if (m_titleBar) {
        root->AddChild(m_titleBar, 0, 0);
    }
    if (m_content) {
        root->AddChild(m_content, 1, 0);
    }

    m_visualRoot = root;
    AddVisualChild(m_visualRoot);
}

}
