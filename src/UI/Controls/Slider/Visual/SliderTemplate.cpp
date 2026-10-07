#include <NeoWPF/UI/Controls/Slider/Slider.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

namespace NeoWPF::UI {

SliderTemplate::SliderTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto track = std::make_shared<Templates::TemplateVisual>("PART_Track");
    track->SetFillColor(NeoWPF::Core::ApplicationThemes::GetPaletteColor(
        NeoWPF::Core::ApplicationThemes::PaletteColor::Track).ToArgb());
    track->SetBorderColor(0x00000000u);
    track->SetBorderThickness(0);

    auto thumb = std::make_shared<Thumb>();
    thumb->SetOrientation(Orientation::Horizontal);
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