#include <NeoWPF/UI/Controls/Base/RangeBase/RangeBase.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI::Properties {
    void RegisterRangeBaseProperties()
    {
        RegisterProperty<UI::RangeBase, double>(
            "Minimum",
            &UI::RangeBase::GetMinimum,
            &UI::RangeBase::SetMinimum);

        RegisterProperty<UI::RangeBase, double>(
            "Maximum",
            &UI::RangeBase::GetMaximum,
            &UI::RangeBase::SetMaximum);

        RegisterProperty<UI::RangeBase, double>(
            "Value",
            &UI::RangeBase::GetValue,
            &UI::RangeBase::SetValue);

        RegisterProperty<UI::RangeBase, double>(
            "SmallChange",
            &UI::RangeBase::GetSmallChange,
            &UI::RangeBase::SetSmallChange);

        RegisterProperty<UI::RangeBase, double>(
            "LargeChange",
            &UI::RangeBase::GetLargeChange,
            &UI::RangeBase::SetLargeChange);
    }
}
