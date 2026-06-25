#pragma once

#include <NeoWPF/UI/Layout/Panel.h>

namespace NeoWPF::UI {

class StackPanel : public Panel {
public:
    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
};

}
