#include <Gallery/Components.h>
#include <Gallery/Panels.h>
#include <NeoWPF/NeoWPF.h>

#include <array>
#include <cstddef>
#include <memory>
#include <vector>

namespace Gallery {
namespace {

using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
using NeoWPF::Core::ApplicationThemes::PaletteBrush;
using NeoWPF::Core::ApplicationThemes::ThemeManager;
using NeoWPF::UI::Button;
using NeoWPF::UI::Grid;
using NeoWPF::UI::Orientation;
using NeoWPF::UI::ScrollBarVisibility;
using NeoWPF::UI::ScrollViewer;
using NeoWPF::UI::StackPanel;

struct TabDefinition {
    const wchar_t* Label;
    Ref<NeoWPF::UI::UIElement> (*CreatePanel)();
};

const std::array<TabDefinition, 3> kTabs{{
    {L"Button", &CreateButtonPanel},
    {L"TextBox", &CreateTextBoxPanel},
    {L"Checkbox", &CreateCheckboxPanel},
}};

Ref<Button> MakeTabButton(const std::wstring& label)
{
    auto button = New<Button>(label);
    button->SetPadding(NeoWPF::UI::Thickness(14, 9, 14, 9));
    return button;
}

void SetTabAppearance(const Ref<Button>& button, bool selected)
{
    const auto background = selected
        ? GetPaletteBrush(PaletteBrush::Accent)
        : GetPaletteBrush(PaletteBrush::Surface);
    const auto foreground = selected
        ? GetPaletteBrush(PaletteBrush::Surface)
        : GetPaletteBrush(PaletteBrush::Foreground);
    const auto border = GetPaletteBrush(PaletteBrush::Border);
    button->SetStylePropertyValue(
        "Background",
        background);
    button->SetStylePropertyValue("Foreground", foreground);
    button->SetStylePropertyValue("BorderBrush", border);

    if (auto backgroundVisual = std::dynamic_pointer_cast<NeoWPF::UI::Templates::TemplateVisual>(
            button->FindTemplateChild("PART_Background"))) {
        backgroundVisual->SetFillColor(background.ToArgb());
        backgroundVisual->SetBorderColor(border.ToArgb());
    }
    button->InvalidateVisual();
}

class GalleryTabHost final : public Grid {
public:
    GalleryTabHost()
    {
        m_themeSubscriptionId = ThemeManager::Instance().SubscribeThemeApplied([this]() {
            RefreshTabAppearances();
        });
    }

    ~GalleryTabHost() override
    {
        ThemeManager::Instance().UnsubscribeThemeApplied(m_themeSubscriptionId);
    }

    void AddTabButton(const Ref<Button>& button)
    {
        m_tabButtons.push_back(button);
    }

    void SetSelectedTab(std::size_t selectedIndex)
    {
        m_selectedTabIndex = selectedIndex;
        RefreshTabAppearances();
    }

private:
    void RefreshTabAppearances()
    {
        for (std::size_t index = 0; index < m_tabButtons.size(); ++index) {
            SetTabAppearance(m_tabButtons[index], index == m_selectedTabIndex);
        }
    }

    std::vector<Ref<Button>> m_tabButtons;
    std::size_t m_selectedTabIndex = 0;
    std::size_t m_themeSubscriptionId = 0;
};

}

Ref<NeoWPF::UI::UIElement> CreateControlGalleryTabs()
{
    auto root = New<GalleryTabHost>();
    root->AddRow(NeoWPF::UI::SizeMode::Auto);
    root->AddRow(NeoWPF::UI::SizeMode::Star);

    auto tabButtons = New<StackPanel>();
    tabButtons->SetOrientation(Orientation::Horizontal);
    tabButtons->SetSpacing(8);

    auto panelViewer = New<ScrollViewer>();
    panelViewer->SetHorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
    panelViewer->SetVerticalScrollBarVisibility(ScrollBarVisibility::Auto);
    for (std::size_t index = 0; index < kTabs.size(); ++index) {
        auto tabButton = MakeTabButton(kTabs[index].Label);
        root->AddTabButton(tabButton);
        tabButton->Click += [index, panelViewer, tabHost = root.get()]() {
            tabHost->SetSelectedTab(index);
            panelViewer->SetContent(kTabs[index].CreatePanel());
        };
        tabButtons->AddChild(tabButton);
    }

    auto tabStrip = New<ScrollViewer>();
    tabStrip->SetHorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
    tabStrip->SetVerticalScrollBarVisibility(ScrollBarVisibility::Disabled);
    tabStrip->SetHeight(52);
    tabStrip->SetContent(tabButtons);

    root->AddChild(tabStrip, 0, 0);
    root->AddChild(panelViewer, 1, 0);
    root->SetSelectedTab(0);
    panelViewer->SetContent(kTabs.front().CreatePanel());
    return root;
}

}