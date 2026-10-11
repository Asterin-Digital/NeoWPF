#include <NeoWPF/UI/Controls/DefaultTitleBar.h>
#include <NeoWPF/Input/Core/RoutedEvent.h>
#include <NeoWPF/Core/Application/Application.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/Core/Application/Resources/ApplicationResources.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

namespace NeoWPF::UI {

DefaultTitleBar::DefaultTitleBar()
{
    m_foregroundBrush = std::make_shared<SolidColorBrush>(0xFF1E1E1Eu);
    ApplyThemeResources();

    SetPadding(0);
    SetBorderThickness(2);
    SetBorderBrush(std::make_shared<SolidColorBrush>(0xFFBDBDBDu));
    SetHeight(32);
    SetVerticalAlignment(VerticalAlignment::Top);

    m_root = std::make_shared<Grid>();
    m_root->AddColumn(SizeMode::Star, 1);
    m_root->AddColumn(SizeMode::Auto, 0);

    m_titleText = std::make_shared<TextBlock>();
    m_titleText->SetText(L"NeoWPF");
    m_titleText->SetForeground(m_foregroundBrush);
    m_titleText->SetFontSize(12);
    m_titleText->SetPadding(Thickness(12, 0, 8, 0));
    m_titleText->SetVerticalAlignment(VerticalAlignment::Center);

    m_minimizeButton = std::make_shared<Button>(L"_");
    m_minimizeButton->SetPadding(Thickness(10, 4, 10, 4));
    m_minimizeButton->SetBackgroundBrush(nullptr);
    m_minimizeButton->SetBorderBrush(nullptr);
    m_minimizeButton->SetForeground(m_foregroundBrush);

    m_maximizeButton = std::make_shared<Button>(L"□");
    m_maximizeButton->SetPadding(Thickness(10, 4, 10, 4));
    m_maximizeButton->SetBackgroundBrush(nullptr);
    m_maximizeButton->SetBorderBrush(nullptr);
    m_maximizeButton->SetForeground(m_foregroundBrush);

    m_closeButton = std::make_shared<Button>(L"X");
    m_closeButton->SetPadding(Thickness(10, 4, 10, 4));
    m_closeButton->SetBackgroundBrush(nullptr);
    m_closeButton->SetBorderBrush(nullptr);
    m_closeButton->SetForeground(m_foregroundBrush);

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
    SetForeground(m_foregroundBrush);
}

void DefaultTitleBar::ApplyThemeResources()
{
    std::shared_ptr<Brush> resourceBrush;
    if (NeoWPF::Core::ApplicationResources::Resources::TryGetBrush(
            "Chrome.BackgroundBrush", resourceBrush)) {
        SetBackgroundBrush(resourceBrush);
    }
    if (NeoWPF::Core::ApplicationResources::Resources::TryGetBrush(
            "Chrome.ForegroundBrush", resourceBrush)) {
        m_foregroundBrush = resourceBrush;
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

void DefaultTitleBar::SetForeground(std::shared_ptr<Brush> brush)
{
    m_foregroundBrush = std::move(brush);
    if (m_titleText) {
        m_titleText->SetForeground(m_foregroundBrush);
    }
    if (m_minimizeButton) {
        m_minimizeButton->SetForeground(m_foregroundBrush);
    }
    if (m_maximizeButton) {
        m_maximizeButton->SetForeground(m_foregroundBrush);
    }
    if (m_closeButton) {
        m_closeButton->SetForeground(m_foregroundBrush);
    }
}

std::shared_ptr<Brush> DefaultTitleBar::GetForeground() const
{
    return m_foregroundBrush;
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
