#include <Gallery/MainWindow.h>
#include <Gallery/Components.h>
#include <NeoWPF/NeoWPF.h>

namespace Gallery {

Ref<NeoWPF::UI::Window> CreateMainWindow()
{
    auto window = New<NeoWPF::UI::Window>();
    window->SetTitle(L"NeoWPF Control Gallery");
    window->SetBackgroundBrush(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Background));
    window->SetContent(CreateControlGalleryTabs());
    return window;
}

}
