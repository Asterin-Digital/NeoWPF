#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
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
    track->SetFillColor(0xFF2A2A2Au);
    track->SetBorderColor(0x00000000u);
    track->SetBorderThickness(0);

    auto thumb = std::make_shared<Templates::TemplateVisual>("PART_Thumb");
    thumb->SetFillColor(0xFF5A5A5Au);
    thumb->SetBorderColor(0x00000000u);
    thumb->SetBorderThickness(0);

    AddTemplateChild("PART_Track", track);
    AddTemplateChild("PART_Thumb", thumb);
}

}
