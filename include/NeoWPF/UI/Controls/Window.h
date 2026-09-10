#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Localization/LocaleTable.h>
#include <memory>
#include <string>

namespace NeoWPF::UI {
class Visual;

class Window : public FrameworkElement {
public:
    static const Properties::DependencyProperty& ContentProperty();
    static const Properties::DependencyProperty& TitleBarProperty();
    static const Properties::DependencyProperty& TitleProperty();
    static const Properties::DependencyProperty& LocalesProperty();

    void SetTitle(const wchar_t* title) { m_title = title ? title : L""; }
    void SetTitle(const std::wstring& title) { m_title = title; }
    const wchar_t* GetTitle() const noexcept { return m_title.c_str(); }
    const std::wstring& GetTitleValue() const noexcept { return m_title; }

    void AddLocale(std::shared_ptr<NeoWPF::UI::Localization::LocaleTable> table);
    const std::vector<std::shared_ptr<NeoWPF::UI::Localization::LocaleTable>>& GetLocales() const noexcept { return m_locales; }
    bool TryResolveLocale(const std::wstring& key, std::wstring& value) const;

    void SetContent(const std::shared_ptr<Visual>& content) {
        m_content = std::move(content);
        RebuildVisualTree();
    }
    std::shared_ptr<Visual> GetContent() const noexcept {
        return m_visualRoot ? m_visualRoot : m_content;
    }

    void SetTitleBar(const std::shared_ptr<Visual>& titleBar) {
        m_titleBar = std::move(titleBar);
        RebuildVisualTree();
    }
    std::shared_ptr<Visual> GetTitleBar() const noexcept { return m_titleBar; }

private:
    void RebuildVisualTree() noexcept;

private:
    std::wstring m_title = L"NeoWPF";
    std::shared_ptr<Visual> m_content;
    std::shared_ptr<Visual> m_titleBar;
    std::shared_ptr<Visual> m_visualRoot;
    std::vector<std::shared_ptr<NeoWPF::UI::Localization::LocaleTable>> m_locales;
};

}
