#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Localization/LocaleTable.h>
#include <memory>
#include <optional>
#include <string>

namespace NeoWPF::UI {
class Visual;
class Popup;

/*
    @summary
    Represents an application window.
*/
class Window : public FrameworkElement {
public:
    static const Properties::DependencyProperty& ContentProperty();
    static const Properties::DependencyProperty& TitleBarProperty();
    static const Properties::DependencyProperty& TitleProperty();
    static const Properties::DependencyProperty& LocalesProperty();
    static const Properties::DependencyProperty& BackgroundColorProperty();

    /*
        @summary
        Sets the window title.

        @param title
        Title text. A null pointer clears the title.
    */
    void SetTitle(const wchar_t* title) { m_title = title ? title : L""; }

    /*
        @summary
        Sets the window title.
    */
    void SetTitle(const std::wstring& title) { m_title = title; }

    /*
        @summary
        Returns the window title.
    */
    const wchar_t* GetTitle() const noexcept { return m_title.c_str(); }

    /*
        @summary
        Returns the window title value.
    */
    const std::wstring& GetTitleValue() const noexcept { return m_title; }

    /*
        @summary
        Sets the renderer clear color for this window.

        @param color
        ARGB clear color.
    */
    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept;

    /*
        @summary
        Sets the renderer clear color from an ARGB value.

        @param color
        ARGB color.
    */
    void SetBackgroundColor(std::uint32_t color) noexcept { SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color)); }

    /*
        @summary
        Returns the effective renderer clear color.
    */
    NeoWPF::UI::Color GetBackgroundColor() const noexcept;

    /*
        @summary
        Adds a locale table to the window.

        @param table
        Locale table to add. Invalid tables are ignored.
    */
    void AddLocale(std::shared_ptr<NeoWPF::UI::Localization::LocaleTable> table);

    /*
        @summary
        Returns the window locale tables.
    */
    const std::vector<std::shared_ptr<NeoWPF::UI::Localization::LocaleTable>>& GetLocales() const noexcept { return m_locales; }

    /*
        @summary
        Resolves a localized string using the window's locale tables.

        @param key
        Localization key.

        @param value
        Resolved value.

        @returns
        True when a value was found; otherwise false.
    */
    bool TryResolveLocale(const std::wstring& key, std::wstring& value) const;

    /*
        @summary
        Sets the main content visual for the window.

        @param content
        Content visual. A null visual clears the content.
    */
    void SetContent(const std::shared_ptr<Visual>& content) {
        m_content = std::move(content);
        RebuildVisualTree();
    }

    /*
        @summary
        Returns the window content visual.
    */
    std::shared_ptr<Visual> GetContent() const noexcept {
        return m_visualRoot ? m_visualRoot : m_content;
    }

    /*
        @summary
        Sets the optional title bar visual.

        @param titleBar
        Title bar visual. A null visual removes the title bar.
    */
    void SetTitleBar(const std::shared_ptr<Visual>& titleBar) {
        m_titleBar = std::move(titleBar);
        RebuildVisualTree();
    }

    /*
        @summary
        Returns the title bar visual.
    */
    std::shared_ptr<Visual> GetTitleBar() const noexcept { return m_titleBar; }

    /*
        @summary
        Adds a visual overlay above the window content.

        @param overlay
        Overlay visual to add. Null and duplicate overlays are ignored.

        @remarks
        Used by popup controls.
    */
    void AddOverlay(const std::shared_ptr<Visual>& overlay);

    /*
        @summary
        Removes a visual overlay from the window.

        @param overlay
        Overlay visual to remove.
    */
    void RemoveOverlay(const std::shared_ptr<Visual>& overlay);

    /*
        @summary
        Returns the window overlays.
    */
    const std::vector<std::shared_ptr<Visual>>& GetOverlays() const noexcept { return m_overlays; }

private:
    void RebuildVisualTree() noexcept;

private:
    std::wstring m_title = L"NeoWPF";
    std::optional<NeoWPF::UI::Color> m_backgroundColor;
    std::shared_ptr<Visual> m_content;
    std::shared_ptr<Visual> m_titleBar;
    std::shared_ptr<Visual> m_visualRoot;
    std::vector<std::shared_ptr<Visual>> m_overlays;
    std::vector<std::shared_ptr<NeoWPF::UI::Localization::LocaleTable>> m_locales;
};

}
