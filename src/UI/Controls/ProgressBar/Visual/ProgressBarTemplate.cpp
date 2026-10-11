#include <NeoWPF/UI/Controls/ProgressBar/ProgressBar.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>

namespace NeoWPF::UI {

ProgressBarTemplate::ProgressBarTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto track = std::make_shared<Templates::TemplateVisual>("PART_Track");
    track->SetFillBrush(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Track));
    track->SetBorderThickness(0);

    auto indicator = std::make_shared<Templates::TemplateVisual>("PART_Indicator");
    indicator->SetFillBrush(NeoWPF::Core::ApplicationThemes::GetPaletteBrush(
        NeoWPF::Core::ApplicationThemes::PaletteBrush::Accent));
    indicator->SetBorderThickness(0);

    root->AddVisualChild(track);
    root->AddVisualChild(indicator);
    AddTemplateChild("PART_Track", track);
    AddTemplateChild("PART_Indicator", indicator);
}

}
