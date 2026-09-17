#pragma once

#include <NeoWPF/NeoWPF.h>

namespace NeoPM::UI::Components {

NeoWPF::Ref<NeoWPF::UI::Popup> MakeDialog(
    NeoWPF::UI::FrameworkElement* placementTarget,
    const NeoWPF::Ref<NeoWPF::UI::UIElement>& content,
    NeoWPF::UI::PopupPlacement placement);

}
