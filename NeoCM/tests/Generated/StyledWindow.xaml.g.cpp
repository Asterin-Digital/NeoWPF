#include <StyledWindow.xaml.g.h>
#include <NeoWPF/Platform/PlatformUtils.h>

namespace NeoCM::Tests {

void StyledWindow::InitializeComponent()
{
    if (Root) {
        return;
    }

    auto builder0 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Grid>();
    builder0.Name("Root");
    auto element0 = builder0.Build();
    auto builder1 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Button>();
    builder1.Name("StyledButton");
    auto element1 = builder1.Build();
    auto builder2 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Styling::Style>();
    auto element2 = builder2.Build();
    auto builder3 = NeoWPF::UI::Builder::Build<NeoWPF::UI::Styling::Triggers::Setter>();
    auto element3 = builder3.Build();

    builder2.Set(NeoWPF::UI::Styling::Style::TargetTypeProperty(), std::type_index(typeid(NeoWPF::UI::Button)));
    builder3.Set(NeoWPF::UI::Styling::Triggers::Setter::PropertyProperty(), "Text");
    builder3.Set(NeoWPF::UI::Styling::Triggers::Setter::ValueProperty(), "Compiled implicit");

    element2->AddSetter(element3);
    element0->SetImplicitXamlStyle(element2->GetTargetType(), element2);

    builder0.AddChild(element1);

    Root = element0;
    StyledButton = element1;
}

}
