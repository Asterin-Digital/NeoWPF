#pragma once

#include <NeoWPF/NeoWPF.h>

namespace NeoPM::UI::Components {

Ref<NeoWPF::UI::Button> MakeButton(
    const std::wstring& text,
    bool isSelectable = false);

}
