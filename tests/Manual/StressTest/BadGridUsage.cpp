#include <NeoWPF/NeoWPF.h>

using namespace NeoWPF;

int main()
{
    Core::Application::Initialize();

    auto window = Core::Application::Current()->CreateWindow(L"Grid Validation Test", 900, 600);

    auto grid = std::make_shared<Grid>();

    grid->AddRow(UI::SizeMode::Auto);
    grid->AddColumn(UI::SizeMode::Auto);

    window.SetContent(grid);

    auto button = std::make_shared<Button>(L"Test");

    // Negative row
    grid->AddChild(button, -1, 0);

    // Negative column
    grid->AddChild(button, 0, -1);

    // Row out of range
    grid->AddChild(button, 100, 0);

    // Column out of range
    grid->AddChild(button, 0, 100);

    // Zero row span
    grid->AddChild(button, 0, 0, 0, 1);

    // Zero column span
    grid->AddChild(button, 0, 0, 1, 0);

    // Negative row span
    grid->AddChild(button, 0, 0, -1, 1);

    // Negative column span
    grid->AddChild(button, 0, 0, 1, -1);

    // Huge row span
    grid->AddChild(button, 0, 0, 100, 1);

    // Huge column span
    grid->AddChild(button, 0, 0, 1, 100);

    // Attached property tests
    Grid::SetRow(button.get(), -1);
    Grid::SetColumn(button.get(), -1);

    Grid::SetRow(button.get(), 100);
    Grid::SetColumn(button.get(), 100);

    Grid::SetRowSpan(button.get(), 0);
    Grid::SetColumnSpan(button.get(), 0);

    Grid::SetRowSpan(button.get(), -5);
    Grid::SetColumnSpan(button.get(), -5);

    Grid::SetRowSpan(button.get(), 100);
    Grid::SetColumnSpan(button.get(), 100);

    return Core::Application::Run();
}