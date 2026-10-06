#include <Gallery/MainWindow.h>
#include <Gallery/Components.h>
#include <NeoWPF/NeoWPF.h>

namespace Gallery {

Ref<NeoWPF::UI::Window> CreateMainWindow()
{
    auto window = New<NeoWPF::UI::Window>();
    window->SetTitle(L"NeoWPF Control Gallery");
    window->SetContent(CreateControlGalleryTabs());
    return window;
}

}
