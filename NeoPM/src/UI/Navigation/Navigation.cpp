#include <NeoPM/UI/Navigation/Navigation.h>
#include <NeoPM/UI/Components/MakeButton.h>
#include <NeoPM/UI/Components/Theme.h>
#include <NeoPM/UI/Project/ProjectDialog.h>
#include <NeoPM/Utils/User.h>

#include <NeoWPF/NeoWPF.h>


namespace NeoPM::UI::Navigation {

using namespace NeoWPF::UI;

Ref<Visual> CreateMainContent()
{
    auto content = New<Grid>();
    content->SetBackgroundBrush(New<SolidColorBrush>(Theme::WindowBackground));
    content->AddRow(SizeMode::Star, 1);
    content->AddColumn(SizeMode::Fixed, 232);
    content->AddColumn(SizeMode::Star, 1);

    auto sidebar = New<StackPanel>();
    sidebar->SetBackgroundBrush(New<SolidColorBrush>(Theme::SidebarBackground));
    sidebar->SetPadding(Thickness(12, 24, 12, 16));

    auto brand = New<TextBlock>(String(L"NeoPM"));
    brand->SetFontSize(20);
    brand->SetFontWeight(700);
    brand->SetForeground(New<SolidColorBrush>(Theme::TextOnSidebar));
    brand->SetWidth(208);
    brand->SetHorizontalAlignment(HorizontalAlignment::Center);
    brand->SetTextHorizontalAlignment(TextHorizontalAlignment::Left);
    brand->SetMargin(Thickness(0, 0, 0, 28));
    sidebar->AddChild(brand);

    auto homeButton = Components::MakeButton(L"Home", true);
    homeButton->SetMargin(Thickness(0, 0, 0, 8));
    homeButton->Click.Invoke();
    sidebar->AddChild(homeButton);

    auto sidebarFrame = New<Border>();
    sidebarFrame->SetBackgroundBrush(New<SolidColorBrush>(Theme::SidebarBackground));
    sidebarFrame->SetBorderBrush(New<SolidColorBrush>(Theme::SidebarBorder));
    sidebarFrame->SetBorderThickness(Thickness(0, 0, 1, 0));
    sidebarFrame->SetChild(sidebar);

    auto mainSurface = New<Border>();
    mainSurface->SetBackgroundBrush(New<SolidColorBrush>(Theme::WindowBackground));
    mainSurface->SetPadding(Thickness(40));

    auto greeting = String(L"Welcome back");
    if (const auto username = Utils::User::GetCurrentUsername())
    {
        greeting += String(L", ");
        greeting += *username;
    }

    auto header = New<Grid>();
    header->SetBackgroundBrush(New<SolidColorBrush>(Theme::WindowBackground));
    header->AddColumn(SizeMode::Star, 1);
    header->AddColumn(SizeMode::Auto, 0);

    auto status = New<TextBlock>(greeting);
    status->SetFontSize(24);
    status->SetForeground(New<SolidColorBrush>(Theme::TextPrimary));

    auto newProjectButton = Components::MakeButton(String(L"+ New Project"));
    newProjectButton->SetMargin(Thickness(16, 0, 0, 0));
    newProjectButton->SetHorizontalAlignment(HorizontalAlignment::Right);
    newProjectButton->SetVerticalAlignment(VerticalAlignment::Top);
    newProjectButton->Click += [newProjectButton]() {
        Project::ShowProjectDialog(
            newProjectButton.get(),
            PopupPlacement::Left);
    };

    header->AddChild(status, 0, 0);
    header->AddChild(newProjectButton, 0, 1);
    mainSurface->SetChild(header);

    content->AddChild(sidebarFrame, 0, 0);
    content->AddChild(mainSurface, 0, 1);
    return content;
}

}
