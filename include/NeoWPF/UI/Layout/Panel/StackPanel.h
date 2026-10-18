/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>

namespace NeoWPF::UI {

/*
    @summary
    Arranges child elements in a single line, either horizontally or vertically.

    @details
    The StackPanel measures and arranges its children one after another along the chosen orientation.
    Use SetOrientation() to choose horizontal or vertical layout, and SetSpacing() to add spacing between children.

    @example
    auto stack = std::make_shared<NeoWPF::UI::StackPanel>();
    stack->SetOrientation(NeoWPF::UI::Orientation::Horizontal);
    stack->SetSpacing(8);
    stack->AddChild(child1);
    stack->AddChild(child2);
*/
class StackPanel : public Panel {
public:
    StackPanel() noexcept {
    }

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    /*
        @summary
        Sets the stacking direction for child layout.

        @param orientation
        Horizontal places children left-to-right and Vertical places them top-to-bottom.
    */
    void SetOrientation(Orientation orientation) noexcept {
        if (m_orientation != orientation) {
            m_orientation = orientation;
            InvalidateMeasure();
            InvalidateArrange();
            InvalidateVisual();
        }
    }

    /*
        @summary
        Returns the current orientation used by the StackPanel.
    */
    Orientation GetOrientation() const noexcept { return m_orientation; }

private:
    Orientation m_orientation = Orientation::Vertical;
};

}
