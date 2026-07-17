#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {

void RegisterCoreControlProperties() {
    RegisterUIElementProperties();
    RegisterFrameworkElementProperties();
    RegisterButtonProperties();
    RegisterCheckboxProperties();
    RegisterItemsControlProperties();
    RegisterScrollBarProperties();
    RegisterTextBlockProperties();
    RegisterTextBoxProperties();
    RegisterBorderProperties();
    RegisterPanelProperties();
}

} // namespace NeoWPF::UI::Properties
