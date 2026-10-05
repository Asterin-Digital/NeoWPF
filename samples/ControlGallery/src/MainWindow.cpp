#include <Gallery/MainWindow.h>
#include <Gallery/Panels.h>
#include <NeoWPF/NeoWPF.h>

namespace Gallery {

Ref<NeoWPF::UI::Window> CreateMainWindow()
{
    auto window = New<NeoWPF::UI::Window>();
    window->SetTitle(L"NeoWPF Control Gallery / Button");
    window->SetContent(CreateButtonPanel());
    return window;
}

}
