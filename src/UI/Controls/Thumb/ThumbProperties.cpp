#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterThumbProperties()
    {
        RegisterProperty<UI::Thumb, int>("DragOffset");
    }
}
