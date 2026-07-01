#include <NeoWPF/UI/Controls/TextBox/TextBox.h>

namespace NeoWPF::UI {

TextBoxTemplate::TextBoxTemplate()
    : Templates::ControlTemplate([](UIElement* element, Templates::ControlTemplateBuilder& builder) {
        (void)builder;

        if (!element) {
            return;
        }

        auto* textBox = dynamic_cast<TextBox*>(element);
        if (!textBox) {
            return;
        }
        (void)textBox;
    })
{
    AddRequiredPart("PART_ContentHost", true);
    AddRequiredPart("PART_Border", false);
}

}
