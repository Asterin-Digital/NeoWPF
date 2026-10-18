/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/NeoWPF.h>
#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>
#include <NeoWPF/UI/Layout/Grid/RowDefinition.h>

namespace NeoCM::Tests {

class MainWindow {
public:
    std::shared_ptr<NeoWPF::UI::Grid> Root;
    std::shared_ptr<NeoWPF::UI::Button> ApplyButton;
    std::shared_ptr<NeoWPF::UI::TextBlock> BoundText;

    void InitializeComponent();
};

}
