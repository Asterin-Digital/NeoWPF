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
    RegisterPopupProperties();
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
    RegisterLocaleStringProperties();
    RegisterLocaleTableProperties();
}

} // namespace NeoWPF::UI::Properties
