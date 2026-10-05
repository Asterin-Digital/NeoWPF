// Passes, layout is the same across NeoWPF and WPF

#include <NeoWPF/NeoWPF.h>
#include <memory>

int main()
{
    NeoWPF::Core::Application::Initialize();

    auto window = NeoWPF::Core::Application::Current()->CreateWindow(L"Settings", 500, 350);

    // Root Grid: one cell to center the card
    auto rootGrid = std::make_shared<NeoWPF::UI::Grid>();
    rootGrid->AddRow(NeoWPF::UI::SizeMode::Star, 1);
    rootGrid->AddColumn(NeoWPF::UI::SizeMode::Star, 1);

    // Main card: Border with padding
    auto card = std::make_shared<NeoWPF::UI::Border>();
    card->SetBackgroundColor(0xFFF5F5F5u);
    card->SetBorderColor(0xFFCCCCCCu);
    card->SetBorderThickness(1);
    card->SetPadding(20);
    card->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);
    card->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);

    // Vertical StackPanel inside the card
    auto stack = std::make_shared<NeoWPF::UI::StackPanel>();

    // ---- Header ----
    auto headerBorder = std::make_shared<NeoWPF::UI::Border>();
    headerBorder->SetBackgroundColor(0xFF3B82F6u);
    headerBorder->SetPadding(12);
    auto headerText = std::make_shared<NeoWPF::TextBlock>();
    headerText->SetText(L"Settings");
    headerText->SetFontSize(20);
    headerText->SetTextColor(0xFFFFFFFFu);
    headerText->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);
    headerBorder->SetChild(headerText);
    stack->AddChild(headerBorder);

    // ---- Form Grid (labels + textboxes) ----
    auto formGrid = std::make_shared<NeoWPF::UI::Grid>();
    formGrid->AddRow(NeoWPF::UI::SizeMode::Auto);  // row 0
    formGrid->AddRow(NeoWPF::UI::SizeMode::Auto);  // row 1
    formGrid->AddRow(NeoWPF::UI::SizeMode::Auto);  // row 2 (for spacing)
    formGrid->AddColumn(NeoWPF::UI::SizeMode::Auto); // col 0 (labels)
    formGrid->AddColumn(NeoWPF::UI::SizeMode::Star, 1); // col 1 (fields)

    // Row 0: Name
    auto nameLabel = std::make_shared<NeoWPF::TextBlock>();
    nameLabel->SetText(L"Name:");
    nameLabel->SetFontSize(14);
    nameLabel->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    formGrid->AddChild(nameLabel, 0, 0);

    auto nameBox = std::make_shared<NeoWPF::TextBox>();
    nameBox->SetText(L"John Doe");
    nameBox->SetPadding(6);
    nameBox->SetWidth(180);
    nameBox->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    formGrid->AddChild(nameBox, 0, 1);

    // Row 1: Email
    auto emailLabel = std::make_shared<NeoWPF::TextBlock>();
    emailLabel->SetText(L"Email:");
    emailLabel->SetFontSize(14);
    emailLabel->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    formGrid->AddChild(emailLabel, 1, 0);

    auto emailBox = std::make_shared<NeoWPF::TextBox>();
    emailBox->SetText(L"john@example.com");
    emailBox->SetPadding(6);
    emailBox->SetWidth(180);
    emailBox->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    formGrid->AddChild(emailBox, 1, 1);

    // Spacer row (row 2) – just a small height Border to add vertical gap
    auto spacer = std::make_shared<NeoWPF::UI::Border>();
    spacer->SetHeight(12);
    spacer->SetBackgroundColor(0x00000000u); // transparent
    formGrid->AddChild(spacer, 2, 0, 1, 2);  // span both columns

    stack->AddChild(formGrid);

    // ---- Footer: two buttons side by side ----
    auto footerStack = std::make_shared<NeoWPF::UI::StackPanel>();
    footerStack->SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    footerStack->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Right);

    auto saveBtn = std::make_shared<NeoWPF::Button>(L"Save");
    saveBtn->SetWidth(80);
    saveBtn->SetHeight(30);
    saveBtn->SetBackgroundColor(0xFF3B82F6u);
    saveBtn->SetTextColor(0xFFFFFFFFu);
    saveBtn->SetPadding(6);
    footerStack->AddChild(saveBtn);

    // Add a small spacer between buttons using a Border with fixed width
    auto btnSpacer = std::make_shared<NeoWPF::UI::Border>();
    btnSpacer->SetWidth(10);
    btnSpacer->SetBackgroundColor(0x00000000u);
    footerStack->AddChild(btnSpacer);

    auto cancelBtn = std::make_shared<NeoWPF::Button>(L"Cancel");
    cancelBtn->SetWidth(80);
    cancelBtn->SetHeight(30);
    cancelBtn->SetBackgroundColor(0xFFEF4444u);
    cancelBtn->SetTextColor(0xFFFFFFFFu);
    cancelBtn->SetPadding(6);
    footerStack->AddChild(cancelBtn);

    stack->AddChild(footerStack);

    // Assemble the card
    card->SetChild(stack);
    rootGrid->AddChild(card, 0, 0);

    window.SetContent(rootGrid);
    window.Resize(500, 350);

    int result = NeoWPF::Core::Application::Run();
    NeoWPF::Core::Application::Shutdown();
    return result;
}

/*
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace Saphire.NET
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            // Title, Width, Height are already set in XAML

            var rootGrid = new Grid();
            rootGrid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) });
            rootGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });

            // Main card
            var card = new Border
            {
                Background = new SolidColorBrush(Color.FromRgb(0xF5, 0xF5, 0xF5)),
                BorderBrush = new SolidColorBrush(Color.FromRgb(0xCC, 0xCC, 0xCC)),
                BorderThickness = new Thickness(1),
                Padding = new Thickness(20),
                HorizontalAlignment = HorizontalAlignment.Center,
                VerticalAlignment = VerticalAlignment.Center
            };

            var stack = new StackPanel();

            // Header
            var headerBorder = new Border
            {
                Background = new SolidColorBrush(Color.FromRgb(0x3B, 0x82, 0xF6)),
                Padding = new Thickness(12)
            };
            var headerText = new TextBlock
            {
                Text = "Settings",
                FontSize = 20,
                Foreground = Brushes.White,
                HorizontalAlignment = HorizontalAlignment.Center
            };
            headerBorder.Child = headerText;
            stack.Children.Add(headerBorder);

            // Form Grid
            var formGrid = new Grid();
            formGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            formGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            formGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto }); // spacer row
            formGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
            formGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });

            // Row 0: Name
            var nameLabel = new TextBlock { Text = "Name:", FontSize = 14, VerticalAlignment = VerticalAlignment.Center };
            Grid.SetRow(nameLabel, 0);
            Grid.SetColumn(nameLabel, 0);
            formGrid.Children.Add(nameLabel);

            var nameBox = new TextBox { Text = "John Doe", Padding = new Thickness(6), Width = 180, VerticalAlignment = VerticalAlignment.Center };
            Grid.SetRow(nameBox, 0);
            Grid.SetColumn(nameBox, 1);
            formGrid.Children.Add(nameBox);

            // Row 1: Email
            var emailLabel = new TextBlock { Text = "Email:", FontSize = 14, VerticalAlignment = VerticalAlignment.Center };
            Grid.SetRow(emailLabel, 1);
            Grid.SetColumn(emailLabel, 0);
            formGrid.Children.Add(emailLabel);

            var emailBox = new TextBox { Text = "john@example.com", Padding = new Thickness(6), Width = 180, VerticalAlignment = VerticalAlignment.Center };
            Grid.SetRow(emailBox, 1);
            Grid.SetColumn(emailBox, 1);
            formGrid.Children.Add(emailBox);

            // Spacer row (transparent border with height)
            var spacer = new Border { Height = 12, Background = Brushes.Transparent };
            Grid.SetRow(spacer, 2);
            Grid.SetColumn(spacer, 0);
            Grid.SetColumnSpan(spacer, 2);
            formGrid.Children.Add(spacer);

            stack.Children.Add(formGrid);

            // Footer buttons
            var footerStack = new StackPanel { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Right };

            var saveBtn = new Button
            {
                Content = "Save",
                Width = 80,
                Height = 30,
                Background = new SolidColorBrush(Color.FromRgb(0x3B, 0x82, 0xF6)),
                Foreground = Brushes.White,
                Padding = new Thickness(6)
            };
            footerStack.Children.Add(saveBtn);

            // Spacer between buttons
            var btnSpacer = new Border { Width = 10, Background = Brushes.Transparent };
            footerStack.Children.Add(btnSpacer);

            var cancelBtn = new Button
            {
                Content = "Cancel",
                Width = 80,
                Height = 30,
                Background = new SolidColorBrush(Color.FromRgb(0xEF, 0x44, 0x44)),
                Foreground = Brushes.White,
                Padding = new Thickness(6)
            };
            footerStack.Children.Add(cancelBtn);

            stack.Children.Add(footerStack);

            card.Child = stack;
            rootGrid.Children.Add(card);

            Content = rootGrid;
        }
    }
}

*/