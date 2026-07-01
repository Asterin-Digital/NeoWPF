#include <NeoWPF/UI/Styling/DefaultStyles.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Layout/Panel/Panel.h>

namespace NeoWPF::UI::Styling {

void RegisterDefaultStyles()
{
    NeoWPF::UI::RegisterTextBlockStyles();
    NeoWPF::UI::RegisterTextBoxStyles();
    NeoWPF::UI::RegisterButtonStyles();
    NeoWPF::UI::RegisterPanelStyles();
}

}
