/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Layout/Placement/Orientation.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <cstdint>

namespace NeoWPF::UI {

/*
    @summary
    Provides a draggable visual used to manipulate a value or position.
*/
class Thumb : public Control {
public:
    Thumb() = default;
    ~Thumb() override = default;

    /*
        @summary
        Sets the direction in which the thumb moves.
    */
    void SetOrientation(Orientation orientation) noexcept { m_orientation = orientation; }
    /*
        @summary
        Returns the direction in which the thumb moves.
    */
    Orientation GetOrientation() const noexcept { return m_orientation; }

    /*
        @summary
        Reports whether the thumb is being dragged.
    */
    bool IsDragging() const noexcept { return m_isDragging; }
    /*
        @summary
        Sets whether the thumb is being dragged.
    */
    void SetDragging(bool dragging) noexcept { m_isDragging = dragging; }
    /*
        @summary
        Sets the thumb's position and dimensions.
    */
    void SetBounds(int x, int y, int width, int height) noexcept;
    /*
        @summary
        Reports whether the thumb can currently be interacted with.
    */
    bool IsInteractable() const noexcept override;

protected:
    /*
        @summary
        Handles routed input for thumb dragging.

        @param args
        The routed event arguments.

        @internal
    */
    void OnRoutedEvent(NeoWPF::Input::Core::RoutedEventArgs* args) override;

public:
    /*
        @summary
        Adds the thumb drawing operations to a command list.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    /*
        @summary
        Fires when dragging begins and provides the starting position along the thumb axis.
    */
    NeoWPF::Core::Event<int> DragStarted;
    /*
        @summary
        Fires as the pointer moves and provides the movement delta along the thumb axis.
    */
    NeoWPF::Core::Event<int> DragDelta;
    /*
        @summary
        Fires when dragging ends and provides the final position along the thumb axis.
    */
    NeoWPF::Core::Event<int> DragCompleted;

private:
    Orientation m_orientation = Orientation::Vertical;
    bool m_isDragging = false;
    bool m_isVisible = true;
    int m_dragStartPointer = 0;
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
};

}
