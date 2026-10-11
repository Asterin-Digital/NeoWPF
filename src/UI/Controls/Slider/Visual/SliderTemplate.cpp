#include <NeoWPF/UI/Controls/Slider/Slider.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

namespace NeoWPF::UI {

SliderTemplate::SliderTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto track = std::make_shared<Templates::TemplateVisual>("PART_Track");
    track->SetFillBrush(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Track));
    track->SetBorderThickness(0);

    auto thumb = std::make_shared<Thumb>();
    thumb->SetOrientation(Orientation::Horizontal);
    thumb->SetBackgroundBrush(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Thumb));
    thumb->SetBorderBrush(nullptr);
    thumb->SetBorderThickness(0);

    root->AddVisualChild(track);
    root->AddVisualChild(thumb);
    AddTemplateChild("PART_Track", track);
    AddTemplateChild("PART_Thumb", thumb);
}

}