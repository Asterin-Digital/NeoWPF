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

class DefaultTitleBar : public Border {
public:
    DefaultTitleBar();
    ~DefaultTitleBar() override = default;

    void SetTitle(const std::wstring& title);
    const std::wstring& GetTitle() const noexcept;

    void SetBackgroundColor(std::uint32_t argbColor) noexcept;
    std::uint32_t GetBackgroundColor() const noexcept;

    void SetTextColor(std::uint32_t argbColor) noexcept;
    std::uint32_t GetTextColor() const noexcept;

    void SetOnMinimizeClicked(std::function<void()> handler);
    void SetOnMaximizeClicked(std::function<void()> handler);
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
