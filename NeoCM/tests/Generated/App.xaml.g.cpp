#include <App.xaml.g.h>
#include <NeoWPF/Platform/PlatformUtils.h>

namespace NeoCM::Tests {

void App::InitializeComponent()
{
    if (IsXamlComponentInitialized()) {
        return;
    }

    auto builder0 = NeoWPF::UI::Builder::ElementBuilder<NeoWPF::Core::Application>(std::shared_ptr<NeoWPF::Core::Application>(this, [](NeoWPF::Core::Application*) {}));
    auto element0 = builder0.Build();




    MarkXamlComponentInitialized();
}

}
