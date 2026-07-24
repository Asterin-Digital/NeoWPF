// Passes, layout is the same between NeoWPF and WPF

#include <NeoWPF/NeoWPF.h>
#include <memory>

int main()
{
    NeoWPF::Core::Application::Initialize();

    auto window = NeoWPF::Core::Application::Current()->CreateWindow(L"Grid Test", 600, 400);

    auto grid = std::make_shared<NeoWPF::UI::Grid>();

    // Define rows: Auto, Star, Auto
    grid->AddRow(NeoWPF::UI::SizeMode::Auto);          // row 0
    grid->AddRow(NeoWPF::UI::SizeMode::Star, 1);       // row 1
    grid->AddRow(NeoWPF::UI::SizeMode::Auto);          // row 2

    // Define columns: Auto, Star, Auto
    grid->AddColumn(NeoWPF::UI::SizeMode::Auto);       // col 0
    grid->AddColumn(NeoWPF::UI::SizeMode::Star, 1);    // col 1
    grid->AddColumn(NeoWPF::UI::SizeMode::Auto);       // col 2

    // ----- Top row: Border with TextBlock spanning all columns -----
    auto titleBorder = std::make_shared<NeoWPF::UI::Border>();
    titleBorder->SetBackgroundColor(0xFF4A90E2u);
    titleBorder->SetPadding(10);

    auto titleText = std::make_shared<NeoWPF::TextBlock>();
    titleText->SetText(L"Grid Test");
    titleText->SetFontSize(24);
    titleText->SetTextColor(0xFFFFFFFFu);
    titleText->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);

    titleBorder->SetChild(titleText);
    grid->AddChild(titleBorder, 0, 0, 1, 3);   // row 0, col 0, span 1 row, 3 columns

    // ----- Middle row: TextBox, Button, Checkbox -----
    auto textBox = std::make_shared<NeoWPF::TextBox>();
    textBox->SetText(L"Enter text");
    textBox->SetWidth(150);
    textBox->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    grid->AddChild(textBox, 1, 0);             // row 1, col 0

    auto buttonMid = std::make_shared<NeoWPF::Button>(L"Click");
    buttonMid->SetWidth(80);
    buttonMid->SetHeight(30);
    buttonMid->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    grid->AddChild(buttonMid, 1, 1);           // row 1, col 1

    auto checkBox = std::make_shared<NeoWPF::Checkbox>();
    checkBox->SetText(L"Enable");
    checkBox->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    grid->AddChild(checkBox, 1, 2);            // row 1, col 2

    // ----- Bottom row: Button spanning columns 1‑2, right aligned -----
    auto submitButton = std::make_shared<NeoWPF::Button>(L"Submit");
    submitButton->SetWidth(100);
    submitButton->SetHeight(30);
    submitButton->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Right);
    grid->AddChild(submitButton, 2, 1, 1, 2);  // row 2, col 1, span 2 columns

    window.SetContent(grid);
    window.Resize(600, 400);

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
            var grid = new Grid();

            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            grid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) });
            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });

            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
            grid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });

            // ----- Top row -----

            var titleBorder = new Border
            {
                Background = new SolidColorBrush(Color.FromRgb(0x4A, 0x90, 0xE2)),
                Padding = new Thickness(10)
            };

            var titleText = new TextBlock
            {
                Text = "Grid Test",
                FontSize = 24,
                Foreground = Brushes.White,
                HorizontalAlignment = HorizontalAlignment.Center
            };

            titleBorder.Child = titleText;

            Grid.SetRow(titleBorder, 0);
            Grid.SetColumn(titleBorder, 0);
            Grid.SetColumnSpan(titleBorder, 3);

            grid.Children.Add(titleBorder);

            // ----- Middle row -----

            var textBox = new TextBox
            {
                Text = "Enter text",
                Width = 150,
                VerticalAlignment = VerticalAlignment.Center
            };

            Grid.SetRow(textBox, 1);
            Grid.SetColumn(textBox, 0);

            grid.Children.Add(textBox);

            var buttonMid = new Button
            {
                Content = "Click",
                Width = 80,
                Height = 30,
                VerticalAlignment = VerticalAlignment.Center
            };

            Grid.SetRow(buttonMid, 1);
            Grid.SetColumn(buttonMid, 1);

            grid.Children.Add(buttonMid);

            var checkBox = new CheckBox
            {
                Content = "Enable",
                VerticalAlignment = VerticalAlignment.Center
            };

            Grid.SetRow(checkBox, 1);
            Grid.SetColumn(checkBox, 2);

            grid.Children.Add(checkBox);

            // ----- Bottom row -----

            var submitButton = new Button
            {
                Content = "Submit",
                Width = 100,
                Height = 30,
                HorizontalAlignment = HorizontalAlignment.Right
            };

            Grid.SetRow(submitButton, 2);
            Grid.SetColumn(submitButton, 1);
            Grid.SetColumnSpan(submitButton, 2);

            grid.Children.Add(submitButton);

            Content = grid;
        }
    }
}
*/