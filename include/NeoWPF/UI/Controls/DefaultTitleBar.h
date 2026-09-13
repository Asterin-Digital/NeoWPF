#pragma once

#include <NeoWPF/Contracts/IWindowChrome.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>

#include <functional>
#include <memory>
#include <string>

namespace NeoWPF::UI {

/*
    @summary
    Provides a default title bar with window controls.
*/
class DefaultTitleBar : public Border {
public:
    DefaultTitleBar();
    ~DefaultTitleBar() override = default;

    /*
        @summary
        Sets the title bar text.

        @param title
        Title text.
    */
    void SetTitle(const std::wstring& title);
    const std::wstring& GetTitle() const noexcept;

    /*
        @summary
        Sets the title bar background color.

        @param argbColor
        ARGB color value.
    */
    void SetBackgroundColor(std::uint32_t argbColor) noexcept;
    std::uint32_t GetBackgroundColor() const noexcept;

    /*
        @summary
        Sets the title text color.

        @param argbColor
        ARGB color value.
    */
    void SetTextColor(std::uint32_t argbColor) noexcept;
    std::uint32_t GetTextColor() const noexcept;

    /*
        @summary
        Sets the callback for the minimize button.

        @param handler
        Callback invoked when the button is clicked.
    */
    void SetOnMinimizeClicked(std::function<void()> handler);

    /*
        @summary
        Sets the callback for the maximize button.

        @param handler
        Callback invoked when the button is clicked.
    */
    void SetOnMaximizeClicked(std::function<void()> handler);

    /*
        @summary
        Sets the callback for the close button.

        @param handler
        Callback invoked when the button is clicked.
    */
    void SetOnCloseClicked(std::function<void()> handler);

private:
    void ApplyThemeResources();

    std::shared_ptr<Grid> m_root;
    std::shared_ptr<TextBlock> m_titleText;
    std::shared_ptr<Button> m_minimizeButton;
    std::shared_ptr<Button> m_maximizeButton;
    std::shared_ptr<Button> m_closeButton;

    std::function<void()> m_onMinimize;
    std::function<void()> m_onMaximize;
    std::function<void()> m_onClose;

    std::wstring m_title = L"NeoWPF";
    std::uint32_t m_backgroundColor = 0xFFFFFFFFu;
    std::uint32_t m_textColor = 0xFF1E1E1Eu;
};

}
