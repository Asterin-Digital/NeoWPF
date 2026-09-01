#include <NeoWPF/UI/Controls/Thumb/Thumb.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    namespace UI = ::NeoWPF::UI;
    using ::NeoWPF::UI::Color;

    void RegisterThumbProperties()
    {
        RegisterProperty<UI::Thumb, int>("DragOffset");
    }
}
