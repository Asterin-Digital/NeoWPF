#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>

namespace NeoWPF::UI {

TextBoxTemplate::TextBoxTemplate()
{
    auto root = std::make_shared<Visual>();
    SetVisualTree(root);

    auto border = std::make_shared<Templates::TemplateVisual>("PART_Border");
    border->SetFillColor(0xFF2F2F2Fu);
    border->SetBorderColor(0xFF666666u);
    border->SetBorderThickness(1);

    auto contentHost = std::make_shared<Templates::TemplateVisual>("PART_ContentHost");
    contentHost->SetFillColor(0x00000000u);
    contentHost->SetBorderColor(0x00000000u);
    contentHost->SetBorderThickness(0);

    AddTemplateChild("PART_Border", border);
    AddTemplateChild("PART_ContentHost", contentHost);
}

}
