#include <RootWindow.xaml.g.h>
#include <NeoWPF/Platform/PlatformUtils.h>

namespace NeoCM::Tests {

void RootWindow::InitializeComponent()
{
    if (IsXamlComponentInitialized()) {
        return;
    }

    auto builder0 = NeoWPF::UI::Builder::ElementBuilder<NeoWPF::UI::Window>(std::shared_ptr<NeoWPF::UI::Window>(this, [](NeoWPF::UI::Window*) {}));
    auto element0 = builder0.Build();
    auto builder1 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Grid>();
    builder1.Name("RootGrid");
    auto element1 = builder1.Build();
    auto builder2 = NeoWPF::UI::Builder::Build<NeoWPF::UI::TextBlock>();
    builder2.Name("Label");
    auto element2 = builder2.Build();

    builder0.Set(NeoWPF::UI::Window::TitleProperty(), std::wstring(L"Generated Window"));
    builder2.Set(NeoWPF::UI::TextBlock::TextProperty(), std::wstring(L"Generated content"));


    builder0.Assign(NeoWPF::UI::Window::ContentProperty(), element1);
    builder1.AddChild(element2);

    RootGrid = element1;
    Label = element2;
    MarkXamlComponentInitialized();
}

}
