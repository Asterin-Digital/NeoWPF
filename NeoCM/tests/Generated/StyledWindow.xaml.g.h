/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/NeoWPF.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Styling/Style.h>
#include <NeoWPF/UI/Styling/Triggers/Setter.h>

namespace NeoCM::Tests {

class StyledWindow {
public:
    std::shared_ptr<NeoWPF::UI::Grid> Root;
    std::shared_ptr<NeoWPF::UI::Button> StyledButton;

    void InitializeComponent();
};

}
