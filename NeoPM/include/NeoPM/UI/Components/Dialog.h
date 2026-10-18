/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/NeoWPF.h>

namespace NeoPM::UI::Components {

NeoWPF::Ref<NeoWPF::UI::Popup> MakeDialog(
    NeoWPF::UI::FrameworkElement* placementTarget,
    const NeoWPF::Ref<NeoWPF::UI::UIElement>& content,
    NeoWPF::UI::PopupPlacement placement);

}
