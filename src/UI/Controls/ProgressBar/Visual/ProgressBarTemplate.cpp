#include <NeoWPF/UI/Controls/ProgressBar/ProgressBar.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/Visual.h>

namespace NeoWPF::UI {

ProgressBarTemplate::ProgressBarTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto track = std::make_shared<Templates::TemplateVisual>("PART_Track");
    track->SetFillColor(0xFFEAEAEAu);
    track->SetBorderColor(0x00000000u);
    track->SetBorderThickness(0);

    auto indicator = std::make_shared<Templates::TemplateVisual>("PART_Indicator");
    indicator->SetFillColor(0xFF4F8CC9u);
    indicator->SetBorderColor(0x00000000u);
    indicator->SetBorderThickness(0);

    root->AddVisualChild(track);
    root->AddVisualChild(indicator);
    AddTemplateChild("PART_Track", track);
    AddTemplateChild("PART_Indicator", indicator);
}

}
