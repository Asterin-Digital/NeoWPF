#include <ResourceWindow.xaml.g.h>
#include <NeoWPF/Platform/PlatformUtils.h>

namespace NeoCM::Tests {

void ResourceWindow::InitializeComponent()
{
    if (Root) {
        return;
    }

    auto builder0 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Border>();
    builder0.Name("Root");
    auto element0 = builder0.Build();
    auto builder1 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Border>();
    builder1.Name("Nested");
    auto element1 = builder1.Build();
    auto builder2 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Border>();
    builder2.Name("Consumer");
    auto element2 = builder2.Build();
    auto builder3 = NeoWPF::UI::Builder::Build<NeoWPF::UI::TextBlock>();
    auto element3 = builder3.Build();
    auto builder4 = NeoWPF::UI::Builder::Build<NeoWPF::UI::TextBlock>();
    auto element4 = builder4.Build();

    builder3.Set(NeoWPF::UI::TextBlock::TextProperty(), std::wstring(L"Inner"));
    builder4.Set(NeoWPF::UI::TextBlock::TextProperty(), std::wstring(L"Outer"));

    element1->SetResource("Accent", element3);
    element0->SetResource("Accent", element4);
    builder2.Assign(NeoWPF::UI::Content::ContentControl::ContentProperty(), element3);

    builder0.Assign(NeoWPF::UI::Content::ContentControl::ContentProperty(), element1);
    builder1.Assign(NeoWPF::UI::Content::ContentControl::ContentProperty(), element2);

    Root = element0;
    Nested = element1;
    Consumer = element2;
}

}
