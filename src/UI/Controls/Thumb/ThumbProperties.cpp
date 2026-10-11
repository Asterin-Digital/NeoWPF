#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;

    void RegisterThumbProperties()
    {
        RegisterProperty<UI::Thumb, int>("DragOffset");
    }
}
