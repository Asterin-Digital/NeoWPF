#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterCoreControlProperties() {
    RegisterUIElementProperties();
    RegisterFrameworkElementProperties();
    RegisterControlProperties();
    RegisterContentControlProperties();
    RegisterButtonBaseProperties();
    RegisterToggleBaseProperties();
    RegisterGroupedToggleBaseProperties();
    RegisterWindowProperties();
    RegisterApplicationProperties();
    RegisterButtonProperties();
    RegisterFilePickerProperties();
    RegisterRangeBaseProperties();
    RegisterThumbProperties();
    RegisterCheckboxProperties();
    RegisterRadioButtonProperties();
    RegisterItemsControlProperties();
    RegisterSelectorProperties();
    RegisterComboBoxProperties();
    RegisterComboBoxItemProperties();
    RegisterPopupProperties();
    RegisterListBoxProperties();
    RegisterListBoxItemProperties();
    RegisterScrollBarProperties();
    RegisterSliderProperties();
    RegisterProgressBarProperties();
    RegisterTextBlockProperties();
    RegisterTextBoxProperties();
    RegisterImageProperties();
    RegisterBorderProperties();
    RegisterPanelProperties();
    RegisterUniformGridProperties();
    RegisterCanvasProperties();
    RegisterGridProperties();
    RegisterRowDefinitionProperties();
    RegisterColumnDefinitionProperties();
    RegisterScrollViewerProperties();
    RegisterStyleProperties();
    RegisterLocaleStringProperties();
    RegisterLocaleTableProperties();
}

}
