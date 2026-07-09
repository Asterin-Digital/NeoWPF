#include <NeoWPF/UI/Properties/Registration/PropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterCoreControlProperties() {
        RegisterButtonProperties();
        RegisterTextBlockProperties();
        RegisterTextBoxProperties();
        RegisterBorderProperties();
        RegisterPanelProperties();
        RegisterScrollBarProperties();
    }
}

