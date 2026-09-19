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
    RegisterFilePickerProperties();
    RegisterRangeBaseProperties();
    RegisterThumbProperties();
    RegisterCheckboxProperties();
    RegisterItemsControlProperties();
    RegisterSelectorProperties();
    RegisterComboBoxProperties();
    RegisterComboBoxItemProperties();
    RegisterPopupProperties();
    RegisterListBoxProperties();
    RegisterListBoxItemProperties();
    RegisterScrollBarProperties();
    RegisterSliderProperties();
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
