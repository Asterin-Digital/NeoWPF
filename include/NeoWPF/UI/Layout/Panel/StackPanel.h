#pragma once

#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>

namespace NeoWPF::UI {

class StackPanel : public Panel {
public:
    StackPanel() noexcept {
        SetOpacity(0.0f);
        SetBackgroundColor(NeoWPF::UI::Color::FromArgb(0x00000000u));
    }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    void SetOrientation(Orientation orientation) noexcept {
        if (m_orientation != orientation) {
            m_orientation = orientation;
            InvalidateMeasure();
            InvalidateArrange();
            InvalidateVisual();
        }
    }

    Orientation GetOrientation() const noexcept { return m_orientation; }

    static bool stackPanelDebugLogging;

private:
    Orientation m_orientation = Orientation::Vertical;
};

}
