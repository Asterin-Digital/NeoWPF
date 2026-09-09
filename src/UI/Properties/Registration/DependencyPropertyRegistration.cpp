#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterCoreControlProperties() {
    RegisterUIElementProperties();
    RegisterFrameworkElementProperties();
    RegisterControlProperties();
    RegisterContentControlProperties();
    RegisterWindowProperties();
    RegisterApplicationProperties();
    RegisterButtonProperties();
    RegisterRangeBaseProperties();
    RegisterThumbProperties();
    RegisterCheckboxProperties();
    RegisterItemsControlProperties();
    RegisterSelectorProperties();
    RegisterListBoxProperties();
    RegisterListBoxItemProperties();
    RegisterScrollBarProperties();
    RegisterTextBlockProperties();
    RegisterTextBoxProperties();
    RegisterImageProperties();
    RegisterBorderProperties();
    RegisterPanelProperties();
    RegisterGridProperties();
    RegisterRowDefinitionProperties();
    RegisterColumnDefinitionProperties();
    RegisterScrollViewerProperties();
    RegisterStyleProperties();
}

} // namespace NeoWPF::UI::Properties
