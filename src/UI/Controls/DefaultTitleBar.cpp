#include <NeoWPF/UI/Controls/DefaultTitleBar.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>

namespace NeoWPF::UI {

DefaultTitleBar::DefaultTitleBar()
{
    ApplyThemeResources();

    SetPadding(0);
    SetBorderThickness(2);
    SetBorderColor(0xFFBDBDBDu);
    SetBackgroundColor(m_backgroundColor);
    SetHeight(32);
    SetVerticalAlignment(VerticalAlignment::Top);

    m_root = std::make_shared<Grid>();
    m_root->AddColumn(SizeMode::Star, 1);
    m_root->AddColumn(SizeMode::Auto, 0);

    m_titleText = std::make_shared<TextBlock>();
    m_titleText->SetText(L"NeoWPF");
    m_titleText->SetTextColor(m_textColor);
    m_titleText->SetFontSize(12);
    m_titleText->SetPadding(Thickness(12, 0, 8, 0));
    m_titleText->SetVerticalAlignment(VerticalAlignment::Center);

    m_minimizeButton = std::make_shared<Button>(L"_");
    m_minimizeButton->SetPadding(Thickness(10, 4, 10, 4));
    m_minimizeButton->SetBackgroundColor(0x00000000u);
    m_minimizeButton->SetBorderColor(0x00000000u);
    m_minimizeButton->SetTextColor(NeoWPF::UI::Color::FromArgb(m_textColor));

    m_maximizeButton = std::make_shared<Button>(L"□");
    m_maximizeButton->SetPadding(Thickness(10, 4, 10, 4));
    m_maximizeButton->SetBackgroundColor(0x00000000u);
    m_maximizeButton->SetBorderColor(0x00000000u);
    m_maximizeButton->SetTextColor(NeoWPF::UI::Color::FromArgb(m_textColor));

    m_closeButton = std::make_shared<Button>(L"X");
    m_closeButton->SetPadding(Thickness(10, 4, 10, 4));
    m_closeButton->SetBackgroundColor(0x00000000u);
    m_closeButton->SetBorderColor(0x00000000u);
    m_closeButton->SetTextColor(NeoWPF::UI::Color::FromArgb(m_textColor));

    m_minimizeButton->AddHandler(NeoWPF::Input::Core::MouseEnterEvent(), [](NeoWPF::UI::UIElement*, NeoWPF::Input::Core::RoutedEventArgs*) {});
    m_minimizeButton->AddHandler(NeoWPF::Input::Core::MouseLeaveEvent(), [](NeoWPF::UI::UIElement*, NeoWPF::Input::Core::RoutedEventArgs*) {});
    m_minimizeButton->Click += [this]() {
        if (m_onMinimize) {
            m_onMinimize();
        }
    };

    m_maximizeButton->AddHandler(NeoWPF::Input::Core::MouseEnterEvent(), [](NeoWPF::UI::UIElement*, NeoWPF::Input::Core::RoutedEventArgs*) {});
    m_maximizeButton->AddHandler(NeoWPF::Input::Core::MouseLeaveEvent(), [](NeoWPF::UI::UIElement*, NeoWPF::Input::Core::RoutedEventArgs*) {});
    m_maximizeButton->Click += [this]() {
        if (m_onMaximize) {
            m_onMaximize();
        }
    };

    m_closeButton->AddHandler(NeoWPF::Input::Core::MouseEnterEvent(), [](NeoWPF::UI::UIElement*, NeoWPF::Input::Core::RoutedEventArgs*) {});
    m_closeButton->AddHandler(NeoWPF::Input::Core::MouseLeaveEvent(), [](NeoWPF::UI::UIElement*, NeoWPF::Input::Core::RoutedEventArgs*) {});
    m_closeButton->Click += [this]() {
        if (m_onClose) {
            m_onClose();
        }
    };

    auto buttonCluster = std::make_shared<StackPanel>();
    buttonCluster->SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    buttonCluster->AddChild(m_minimizeButton);
    buttonCluster->AddChild(m_maximizeButton);
    buttonCluster->AddChild(m_closeButton);

    m_root->AddChild(m_titleText, 0, 0);
    m_root->AddChild(buttonCluster, 0, 1);
    SetChild(m_root);

    SetTitle(m_title);
    SetBackgroundColor(m_backgroundColor);
    SetTextColor(m_textColor);
}

void DefaultTitleBar::ApplyThemeResources()
{
    NeoWPF::UI::Color resourceColor;
    if (NeoWPF::Core::ApplicationResources::Resources::TryGetColor(
            "Chrome.BackgroundColor", resourceColor)) {
        m_backgroundColor = resourceColor.ToArgb();
    }
    if (NeoWPF::Core::ApplicationResources::Resources::TryGetColor(
            "Chrome.ForegroundColor", resourceColor)) {
        m_textColor = resourceColor.ToArgb();
    }
}

void DefaultTitleBar::SetTitle(const std::wstring& title)
{
    m_title = title.empty() ? L"NeoWPF" : title;
    if (m_titleText) {
        m_titleText->SetText(m_title);
    }
}

const std::wstring& DefaultTitleBar::GetTitle() const noexcept
{
    return m_title;
}

void DefaultTitleBar::SetBackgroundColor(std::uint32_t argbColor) noexcept
{
    m_backgroundColor = argbColor;
    Border::SetBackgroundColor(argbColor);
}

std::uint32_t DefaultTitleBar::GetBackgroundColor() const noexcept
{
    return m_backgroundColor;
}

void DefaultTitleBar::SetTextColor(std::uint32_t argbColor) noexcept
{
    m_textColor = argbColor;
    if (m_titleText) {
        m_titleText->SetTextColor(argbColor);
    }
    if (m_minimizeButton) {
        m_minimizeButton->SetTextColor(NeoWPF::UI::Color::FromArgb(argbColor));
    }
    if (m_maximizeButton) {
        m_maximizeButton->SetTextColor(NeoWPF::UI::Color::FromArgb(argbColor));
    }
    if (m_closeButton) {
        m_closeButton->SetTextColor(NeoWPF::UI::Color::FromArgb(argbColor));
    }
}

std::uint32_t DefaultTitleBar::GetTextColor() const noexcept
{
    return m_textColor;
}

void DefaultTitleBar::SetOnMinimizeClicked(std::function<void()> handler)
{
    m_onMinimize = std::move(handler);
}

void DefaultTitleBar::SetOnMaximizeClicked(std::function<void()> handler)
{
    m_onMaximize = std::move(handler);
}

void DefaultTitleBar::SetOnCloseClicked(std::function<void()> handler)
{
    m_onClose = std::move(handler);
}

}
