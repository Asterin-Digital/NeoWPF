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
using NeoWPF::UI::Border;
using NeoWPF::UI::Grid;
using NeoWPF::UI::SizeMode;
using NeoWPF::UI::ScrollBarVisibility;
using NeoWPF::UI::ScrollViewer;
using NeoWPF::UI::StackPanel;
using NeoWPF::UI::TextBlock;

struct TabDefinition {
    const wchar_t* Label;
    const wchar_t* Description;
    Ref<NeoWPF::UI::UIElement> (*CreatePanel)();
};

const std::array<TabDefinition, 6> kTabs{{
    {L"Buttons", L"Actions, states, and theme response", &CreateButtonPanel},
    {L"Text input", L"Editable fields and read-only values", &CreateTextBoxPanel},
    {L"Checkboxes", L"Independent choices and checked states", &CreateCheckboxPanel},
    {L"Toggles", L"Settings switches and live state changes", &CreateToggleSwitchPanel},
    {L"Selection", L"Grouped choices and item selectors", &CreateSelectionPanel},
    {L"Range & progress", L"Adjustable values and progress feedback", &CreateRangePanel},
}};

Ref<TextBlock> MakeHeaderText(const std::wstring& text, int fontSize, int fontWeight)
{
    auto label = New<TextBlock>(text);
    label->SetFontSize(fontSize);
    label->SetFontWeight(fontWeight);
    return label;
}

Ref<TextBlock> MakeMutedText(const std::wstring& text, int fontSize, int fontWeight)
{
    auto label = MakeHeaderText(text, fontSize, fontWeight);
    label->SetForeground(GetPaletteBrush(PaletteBrush::MutedForeground));
    return label;
}

Ref<Button> MakeTabButton(const std::wstring& label)
{
    auto button = New<Button>(label);
    button->SetWidth(204);
    button->SetPadding(NeoWPF::UI::Thickness(14, 12, 14, 12));
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
    const auto border = selected
        ? GetPaletteBrush(PaletteBrush::Accent)
        : GetPaletteBrush(PaletteBrush::Surface);
    button->SetStylePropertyValue("Background", background);
    button->SetStylePropertyValue("Foreground", foreground);
    button->SetStylePropertyValue("BorderBrush", border);

    if (auto backgroundVisual = std::dynamic_pointer_cast<NeoWPF::UI::Templates::TemplateVisual>(
            button->FindTemplateChild("PART_Background"))) {
        backgroundVisual->SetFillBrush(background);
        backgroundVisual->SetBorderBrush(border);
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
    root->AddColumn(SizeMode::Fixed, 248);
    root->AddColumn(SizeMode::Star, 1);
    root->AddRow(SizeMode::Star, 1);

    auto sidebarContent = New<StackPanel>();
    sidebarContent->SetSpacing(28);

    auto brand = New<StackPanel>();
    brand->SetSpacing(5);
    brand->AddChild(MakeHeaderText(L"NeoWPF", 26, 700));
    brand->AddChild(MakeMutedText(L"CONTROL GALLERY", 11, 650));
    sidebarContent->AddChild(brand);

    auto navigation = New<StackPanel>();
    navigation->SetSpacing(7);
    navigation->AddChild(MakeMutedText(L"COMPONENTS", 11, 650));

    auto panelViewer = New<ScrollViewer>();
    panelViewer->SetHorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
    panelViewer->SetVerticalScrollBarVisibility(ScrollBarVisibility::Auto);

    auto pageTitle = MakeHeaderText(kTabs.front().Label, 30, 650);
    auto pageDescription = MakeMutedText(kTabs.front().Description, 14, 400);
    auto pageIndex = MakeMutedText(L"01 / 06", 13, 600);

    for (std::size_t index = 0; index < kTabs.size(); ++index) {
        auto tabButton = MakeTabButton(kTabs[index].Label);
        root->AddTabButton(tabButton);
        tabButton->Click += [index, panelViewer, pageTitle, pageDescription, pageIndex,
                             tabHost = root.get()]() {
            tabHost->SetSelectedTab(index);
            pageTitle->SetText(kTabs[index].Label);
            pageDescription->SetText(kTabs[index].Description);
            std::wstring indexText = index < 9 ? L"0" : L"";
            indexText += std::to_wstring(index + 1);
            indexText += L" / 06";
            pageIndex->SetText(indexText);
            panelViewer->SetContent(kTabs[index].CreatePanel());
        };
        navigation->AddChild(tabButton);
    }

    sidebarContent->AddChild(navigation);

    auto sidebar = New<Border>();
    sidebar->SetPadding(NeoWPF::UI::Thickness(20, 28, 20, 24));
    sidebar->SetBackgroundBrush(GetPaletteBrush(PaletteBrush::Surface));
    sidebar->SetBorderBrush(GetPaletteBrush(PaletteBrush::Border));
    sidebar->SetBorderThickness(NeoWPF::UI::Thickness(0, 0, 1, 0));
    sidebar->SetChild(sidebarContent);

    auto pageHeading = New<StackPanel>();
    pageHeading->SetSpacing(8);
    pageHeading->AddChild(MakeMutedText(L"NEOWPF / CONTROLS", 11, 650));
    pageHeading->AddChild(pageTitle);
    pageHeading->AddChild(pageDescription);

    auto headerLayout = New<Grid>();
    headerLayout->AddColumn(SizeMode::Star, 1);
    headerLayout->AddColumn(SizeMode::Auto);
    headerLayout->AddRow(SizeMode::Auto);
    headerLayout->AddChild(pageHeading, 0, 0);
    headerLayout->AddChild(pageIndex, 0, 1);

    auto header = New<Border>();
    header->SetPadding(NeoWPF::UI::Thickness(32, 24, 32, 24));
    header->SetBackgroundBrush(GetPaletteBrush(PaletteBrush::Surface));
    header->SetBorderBrush(GetPaletteBrush(PaletteBrush::Border));
    header->SetBorderThickness(NeoWPF::UI::Thickness(0, 0, 0, 1));
    header->SetChild(headerLayout);

    auto contentSurface = New<Border>();
    contentSurface->SetPadding(NeoWPF::UI::Thickness(32));
    contentSurface->SetBackgroundBrush(GetPaletteBrush(PaletteBrush::Background));
    contentSurface->SetChild(panelViewer);

    auto mainContent = New<Grid>();
    mainContent->AddColumn(SizeMode::Star, 1);
    mainContent->AddRow(SizeMode::Auto);
    mainContent->AddRow(SizeMode::Star, 1);
    mainContent->AddChild(header, 0, 0);
    mainContent->AddChild(contentSurface, 1, 0);

    root->AddChild(sidebar, 0, 0);
    root->AddChild(mainContent, 0, 1);
    root->SetSelectedTab(0);
    panelViewer->SetContent(kTabs.front().CreatePanel());
    return root;
}

}