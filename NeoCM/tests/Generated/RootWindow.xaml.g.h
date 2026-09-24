/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/NeoWPF.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Controls/Window.h>
#include <NeoWPF/UI/Layout/Grid/Grid.h>

namespace NeoCM::Tests {

class RootWindow : public NeoWPF::UI::Window {
public:
    RootWindow();
    std::shared_ptr<NeoWPF::UI::Grid> RootGrid;
    std::shared_ptr<NeoWPF::UI::TextBlock> Label;

    void InitializeComponent();
};

}
