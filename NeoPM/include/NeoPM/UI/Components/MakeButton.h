/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/NeoWPF.h>

namespace NeoPM::UI::Components {

Ref<NeoWPF::UI::Button> MakeButton(
    const std::wstring& text,
    bool isSelectable = false);

}
