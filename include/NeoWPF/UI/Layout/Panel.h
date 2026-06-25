#pragma once

#include <NeoWPF/UI/Core/FrameworkElement.h>

namespace NeoWPF::UI {

class Panel : public FrameworkElement {
public:
    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
};

}
