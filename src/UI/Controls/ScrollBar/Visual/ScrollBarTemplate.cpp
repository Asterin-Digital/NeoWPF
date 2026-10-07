#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <memory>

namespace NeoWPF::UI {

ScrollBarTemplate::ScrollBarTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto track = std::make_shared<Templates::TemplateVisual>("PART_Track");
    track->SetFillColor(NeoWPF::Core::ApplicationThemes::GetPaletteColor(
        NeoWPF::Core::ApplicationThemes::PaletteColor::Track).ToArgb());
    track->SetBorderColor(0x00000000u);
    track->SetBorderThickness(0);

    auto thumb = std::make_shared<Thumb>();
    thumb->SetOrientation(Orientation::Vertical);
    thumb->SetBackgroundColor(NeoWPF::Core::ApplicationThemes::GetPaletteColor(
        NeoWPF::Core::ApplicationThemes::PaletteColor::Thumb));
    thumb->SetBorderColor(0x00000000u);
    thumb->SetBorderThickness(0);

    root->AddVisualChild(track);
    root->AddVisualChild(thumb);

    AddTemplateChild("PART_Track", track);
    AddTemplateChild("PART_Thumb", thumb);
}

}
