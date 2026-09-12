#include <MainWindow.xaml.g.h>
#include <NeoWPF/Platform/PlatformUtils.h>

namespace NeoCM::Tests {

void MainWindow::InitializeComponent()
{
    if (Root) {
        return;
    }

    auto builder0 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Grid>();
    builder0.Name("RootGrid");
    auto element0 = builder0.Build();
    auto builder1 = NeoWPF::UI::Builder::Build<NeoWPF::UI::RowDefinition>();
    auto element1 = builder1.Build();
    auto builder2 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Button>();
    builder2.Name("ApplyButton");
    auto element2 = builder2.Build();
    auto builder3 = NeoWPF::UI::Builder::Build<NeoWPF::UI::TextBlock>();
    builder3.Name("BoundText");
    auto element3 = builder3.Build();

    builder2.Set(NeoWPF::UI::Grid::RowProperty(), 2);
    builder2.Set(NeoWPF::UI::Button::TextProperty(), std::wstring(L"Apply"));

    NeoWPF::UI::Data::BindingOperations::SetBinding(*element3, NeoWPF::UI::TextBlock::TextProperty(), NeoWPF::UI::Data::Binding("Title", NeoWPF::UI::Data::BindingMode::TwoWay));

    builder0.Assign(NeoWPF::UI::Grid::RowDefinitionsProperty(), element1);
    builder0.AddChild(element2);
    builder0.AddChild(element3);

    Root = element0;
    ApplyButton = element2;
    BoundText = element3;
}

}
