#pragma once

#include <NeoWPF/NeoWPF.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Layout/Border/Border.h>

namespace NeoCM::Tests {

class ResourceWindow {
public:
    std::shared_ptr<NeoWPF::UI::Border> Root;
    std::shared_ptr<NeoWPF::UI::Border> Nested;
    std::shared_ptr<NeoWPF::UI::Border> Consumer;

    void InitializeComponent();
};

}
