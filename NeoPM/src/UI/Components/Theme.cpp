#include <NeoPM/UI/Components/Theme.h>

#include <NeoWPF/NeoWPF.h>

namespace NeoPM::UI::Theme {

void Register()
{
    NeoWPF::ThemeManager::Instance().RegisterTheme({
        "NeoPM",
        {
            {"NeoPM.WindowBackground", NeoWPF::UI::Color::FromArgb(WindowBackground)},
            {"NeoPM.SidebarBackground", NeoWPF::UI::Color::FromArgb(SidebarBackground)},
            {"NeoPM.Accent", NeoWPF::UI::Color::FromArgb(Accent)}
        }
    });
    NeoWPF::ThemeManager::Instance().SetActiveTheme("NeoPM");
}

}
