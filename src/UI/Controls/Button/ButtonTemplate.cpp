#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <memory>

namespace NeoWPF::UI {

ButtonTemplate::ButtonTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto background = std::make_shared<Templates::TemplateVisual>("PART_Background");
    background->SetFillColor(0xFF2F2F2Fu);
    background->SetBorderColor(0xFF333333u);
    background->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetFillColor(0x00000000u);
    contentHost->SetBorderColor(0x00000000u);
    contentHost->SetBorderThickness(0);

    AddTemplateChild("PART_Background", background);
    AddTemplateChild("PART_ContentHost", contentHost);
}

}
