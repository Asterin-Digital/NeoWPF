#pragma once

#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/Input/Core/Events.h>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <cstdint>
#include <memory>

namespace NeoWPF::UI {

enum class ScrollBarOrientation {
    Vertical,
    Horizontal
};

class ScrollBarStyle;
void RegisterScrollBarStyles();

/*
    @summary
    A scrollbar for vertical or horizontal scrolling.

    @details
    Drop this on a panel or content surface when you want scrolling.
    @example
    auto scrollBar = std::make_shared<NeoWPF::UI::ScrollBar>();
    scrollBar->SetOrientation(ScrollBarOrientation::Vertical);
    scrollBar->SetScrollTarget(contentHost);
    scrollBar->SetViewportElement(viewport);
*/
class ScrollBar : public NeoWPF::UI::Content::ContentControl {
public:
    ScrollBar();

    ~ScrollBar();

    /*
        @summary
        Chooses whether the scrollbar runs vertically or horizontally.

        @param orientation
        The direction the thumb should travel.

        @example
        scrollBar->SetOrientation(ScrollBarOrientation::Horizontal);
    */
    void SetOrientation(ScrollBarOrientation orientation) noexcept { m_orientation = orientation; InvalidateVisual(); }

    /*
        @summary
        Returns the current scrollbar direction.
    */
    ScrollBarOrientation GetOrientation() const noexcept { return m_orientation; }

    /*
        @summary
        Sets the track color behind the thumb.

        @param color
        ARGB color value.
    */
    void SetTrackColor(uint32_t color) noexcept { m_trackColor = color; InvalidateVisual(); }

    /*
        @summary
        Returns the current track color.
    */
    uint32_t GetTrackColor() const noexcept { return m_trackColor; }

    /*
        @summary
        Sets the thumb color.

        @param color
        ARGB color value.
    */
    void SetThumbColor(uint32_t color) noexcept { m_thumbColor = color; InvalidateVisual(); }

    /*
        @summary
        Returns the current thumb color.
    */
    uint32_t GetThumbColor() const noexcept { return m_thumbColor; }

    /*
        @summary
        Sets the thumb size in pixels.

        @param size
        A minimum-sized thumb; values below 1 are clamped to 1.
    */
    void SetThumbSize(int size) noexcept { m_thumbSize = size < 1 ? 1 : size; InvalidateVisual(); }

    /*
        @summary
        Returns the current thumb size.
    */
    int GetThumbSize() const noexcept { return m_thumbSize; }

    /*
        @summary
        Updates the total scrollable content size.

        @param size
        Total size of the content in scroll direction.

        @internal
    */
    void SetContentSize(int size) noexcept;

    /*
        @summary
        Updates the visible viewport size.

        @param size
        Visible size of the viewport in scroll direction.

        @internal
    */
    void SetViewportSize(int size) noexcept;

    /*
        @summary
        Wires the scrollbar to the element that actually scrolls.

        @param target
        The content element whose position should be offset by the scrollbar value.

        @example
        scrollBar->SetScrollTarget(contentHost);
    */
    void SetScrollTarget(const std::shared_ptr<FrameworkElement>& target) noexcept { m_scrollTarget = target; UpdateScrollState(); }

    /*
        @summary
        Wires the scrollbar to the visible viewport element.

        @param viewportElement
        The element that represents the visible area the user sees.

        @example
        scrollBar->SetViewportElement(viewport);
    */
    void SetViewportElement(const std::shared_ptr<FrameworkElement>& viewportElement) noexcept { m_viewportElement = viewportElement; UpdateScrollState(); }

    /*
        @summary
        Sets the current scroll position.

        @param value
        The new value, clamped to the available scroll range.

        @example
        scrollBar->SetValue(240);
    */
    void SetValue(int value) noexcept;

    /*
        @summary
        Returns the current scroll position.
    */
    int GetValue() const noexcept { return m_value; }

    /*
        @summary
        Raised whenever the scroll position changes.

        @details
        Hook this up if you want to react to the thumb moving, such as updating other UI.
    */
    NeoWPF::Core::Event<int> ValueChanged;

    /*
        @summary
        Measures the scrollbar's desired size.

        @internal
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Arranges the scrollbar into its final bounds.

        @internal
    */
    void Arrange(int finalWidth, int finalHeight) override;

    /*
        @summary
        Builds the render commands for the track and thumb.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    /*
        @summary
        Handles mouse-down input for dragging the thumb.

        @internal
    */
    void OnMouseDown(int x, int y, std::uint32_t buttonState) override;

    /*
        @summary
        Handles mouse-up input when dragging ends.

        @internal
    */
    void OnMouseUp(int x, int y, std::uint32_t buttonState) override;

    /*
        @summary
        Handles mouse movement while dragging the thumb.

        @internal
    */
    void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) override;

    /*
        @summary
        Computes the available scroll range.

        @internal
    */
    int GetScrollRange() const noexcept;

    /*
        @summary
        Computes the visual thumb length.

        @internal
    */
    int GetThumbLength(int trackLength) const noexcept;

    /*
        @summary
        Computes the thumb's current position along the track.

        @internal
    */
    int GetThumbPosition(int trackLength, int thumbLength) const noexcept;

private:
    static bool s_scrollBarDebug;

    /*
        @summary
        Updates the current value based on a pointer position.

        @internal
    */
    void UpdateValueFromPosition(int x, int y) noexcept;

    /*
        @summary
        Pushes the current scroll value into the scroll target.

        @internal
    */
    void UpdateScrollTarget() noexcept;

    /*
        @summary
        Recomputes content and viewport size, then updates the target.

        @internal
    */
    void UpdateScrollState() noexcept;

    ScrollBarOrientation m_orientation = ScrollBarOrientation::Vertical;
    uint32_t m_trackColor = 0xFF2A2A2Au;
    uint32_t m_thumbColor = 0xFF5A5A5Au;
    int m_thumbSize = 24;
    int m_contentSize = 0;
    int m_viewportSize = 0;
    int m_value = 0;
    bool m_isDragging = false;
    int m_dragStartValue = 0;
    int m_dragStartPointer = 0;
    std::shared_ptr<FrameworkElement> m_scrollTarget;
    std::shared_ptr<FrameworkElement> m_viewportElement;
};

class ScrollBarTemplate : public Templates::ControlTemplate {
public:
    ScrollBarTemplate();
};

}
