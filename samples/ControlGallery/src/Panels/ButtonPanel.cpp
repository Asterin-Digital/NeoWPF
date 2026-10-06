#include <Gallery/Panels.h>
#include <NeoWPF/NeoWPF.h>

namespace Gallery {
namespace {

using NeoWPF::Core::ApplicationThemes::ThemeManager;

}

Ref<NeoWPF::UI::Border> CreateButtonPanel()
{
    auto panel = New<NeoWPF::UI::Border>();
    panel->SetPadding(24);

    auto button = New<NeoWPF::UI::Button>(L"Toggle theme");
    button->Click += []() {
        ThemeManager::Instance().SetActiveTheme(
            ThemeManager::Instance().GetActiveThemeName() == "Light Mint" ? "Dark Mint" : "Light Mint");
    };

    panel->SetChild(button);
    return panel;
}

}
