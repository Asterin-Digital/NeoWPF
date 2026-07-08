#pragma once

#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Layout/Alignment/Alignment.h>
#include <algorithm>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

class ScrollBar;

/*
    @summary
    Controls how a framework element resolves its width or height.
*/
enum class LayoutSizeMode {
    Auto,
    Stretch
};

class FrameworkElement : public UIElement {
    friend class ScrollBar;
public:
    /*
        @summary
        Returns the size the element wants during measurement.

        @internal
    */
    int GetDesiredWidth() const noexcept { return m_desiredWidth; }

    /*
        @summary
        Returns the size the element wants during measurement.

        @internal
    */
    int GetDesiredHeight() const noexcept { return m_desiredHeight; }

    /*
        @summary
        Sets the size the element reports during measurement.

        @param width
        The desired width.

        @param height
        The desired height.

        @internal
    */
    void SetDesiredSize(int width, int height) noexcept {
        m_desiredWidth = width;
        m_desiredHeight = height;
    }

    /*
        @summary
        Measures the element using the available space.

        @param availableWidth
        The width available to the element.

        @param availableHeight
        The height available to the element.

        @internal
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Places the element in its final bounds.

        @param finalWidth
        The width assigned to the element.

        @param finalHeight
        The height assigned to the element.

        @internal
    */
    void Arrange(int finalWidth, int finalHeight) override;

    /*
        @summary
        Returns the width assigned during arrange.

        @internal
    */
    int GetFinalWidth() const noexcept { return m_finalWidth; }

    /*
        @summary
        Returns the height assigned during arrange.

        @internal
    */
    int GetFinalHeight() const noexcept { return m_finalHeight; }

    /*
        @summary
        Returns whether the element clips its rendered content to its own bounds.
    */
    bool IsClipToBounds() const noexcept { return m_clipToBounds; }

    /*
        @summary
        Enables or disables clipping of the element's rendered content to its own bounds.
    */
    void SetClipToBounds(bool clipToBounds) noexcept {
        if (m_clipToBounds != clipToBounds) {
            m_clipToBounds = clipToBounds;
            InvalidateVisual();
        }
    }

    /*
        @summary
        Returns the clip rectangle for the element's current arranged bounds.

        @internal 
    */
    NeoWPF::Rendering::ClipRect GetClipRect() const noexcept {
        NeoWPF::Rendering::ClipRect clip{};
        if (m_clipToBounds && m_finalWidth > 0 && m_finalHeight > 0) {
            int absX = 0;
            int absY = 0;
            GetAbsolutePosition(absX, absY);

            clip.x = static_cast<float>(absX);
            clip.y = static_cast<float>(absY);
            clip.w = static_cast<float>(m_finalWidth);
            clip.h = static_cast<float>(m_finalHeight);
            clip.active = true;
        }

        auto parent = dynamic_cast<FrameworkElement*>(GetParent());
        if (parent) {
            const auto parentClip = parent->GetClipRect();
            if (parentClip.active) {
                if (!clip.active) {
                    return parentClip;
                }

                const float left = std::max(clip.x, parentClip.x);
                const float top = std::max(clip.y, parentClip.y);
                const float right = std::min(clip.x + clip.w, parentClip.x + parentClip.w);
                const float bottom = std::min(clip.y + clip.h, parentClip.y + parentClip.h);
                if (right <= left || bottom <= top) {
                    return NeoWPF::Rendering::ClipRect{};
                }

                return NeoWPF::Rendering::ClipRect{left, top, right - left, bottom - top, true};
            }
        }

        return clip;
    }

    /*
        @summary
        Moves the element in layout coordinates.

        @param x
        The new X offset.

        @param y
        The new Y offset.

        @internal
    */
    void SetLayoutOffset(int x, int y) noexcept { m_offsetX = x; m_offsetY = y; InvalidateVisual(); }

    /*
        @summary
        Returns the X offset used for layout placement.

        @internal
    */
    int GetLayoutOffsetX() const noexcept { return m_offsetX; }

    /*
        @summary
        Returns the Y offset used for layout placement.

        @internal
    */
    int GetLayoutOffsetY() const noexcept { return m_offsetY; }

    /*
        @summary
        Animates a size change from one rectangle to another.

        @param fromWidth
        The starting width.

        @param toWidth
        The target width.

        @param fromHeight
        The starting height.

        @param toHeight
        The target height.

        @param durationSeconds
        The animation duration in seconds.
    */
    void BeginAnimation(int fromWidth, int toWidth, int fromHeight, int toHeight, float durationSeconds);

    /*
        @summary
        Animates the element toward a target alignment inside its parent.

        @param horizontal
        The target horizontal alignment.

        @param vertical
        The target vertical alignment.

        @param durationSeconds
        The animation duration in seconds.
    */
    void BeginAnimation(HorizontalAlignment horizontal, VerticalAlignment vertical, float durationSeconds);

    /*
        @summary
        Returns the width sizing mode.
    */
    LayoutSizeMode GetWidthMode() const noexcept { return m_widthMode; }

    /*
        @summary
        Sets the width sizing mode.

        @param mode
        The new width mode.

        @example
        element->SetWidthMode(NeoWPF::UI::LayoutSizeMode::Stretch);
    */
    void SetWidthMode(LayoutSizeMode mode) noexcept { m_widthMode = mode; InvalidateVisual(); }

    /*
        @summary
        Returns the height sizing mode.
    */
    LayoutSizeMode GetHeightMode() const noexcept { return m_heightMode; }

    /*
        @summary
        Sets the height sizing mode.

        @param mode
        The new height mode.

        @example
        element->SetHeightMode(NeoWPF::UI::LayoutSizeMode::Auto);
    */
    void SetHeightMode(LayoutSizeMode mode) noexcept { m_heightMode = mode; InvalidateVisual(); }

    /*
        @summary
        Sets a preferred width for the element.

        @param width
        The width to use.
    */
    void SetWidth(int width) noexcept { m_width = width < 0 ? 0 : width; InvalidateVisual(); }

    /*
        @summary
        Sets a preferred height for the element.

        @param height
        The height to use.
    */
    void SetHeight(int height) noexcept { m_height = height < 0 ? 0 : height; InvalidateVisual(); }

    /*
        @summary
        Resolves the final width for the current sizing mode.

        @param availableWidth
        The width available to the element.

        @param desiredWidth
        The width the element wants.

        @internal
    */
    int ResolveWidth(int availableWidth, int desiredWidth) const;

    /*
        @summary
        Resolves the final height for the current sizing mode.

        @param availableHeight
        The height available to the element.

        @param desiredHeight
        The height the element wants.

        @internal
    */
    int ResolveHeight(int availableHeight, int desiredHeight) const;

    /*
        @summary
        Returns the horizontal alignment used when the element is placed in a parent.
    */
    HorizontalAlignment GetHorizontalAlignment() const noexcept { return m_horizontalAlignment; }

    /*
        @summary
        Sets the horizontal alignment used when the element is placed in a parent.

        @param align
        The alignment to use.

        @example
        element->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);
    */
    void SetHorizontalAlignment(HorizontalAlignment align) noexcept { m_horizontalAlignment = align; }

    /*
        @summary
        Returns the vertical alignment used when the element is placed in a parent.
    */
    VerticalAlignment GetVerticalAlignment() const noexcept { return m_verticalAlignment; }

    /*
        @summary
        Sets the vertical alignment used when the element is placed in a parent.

        @param align
        The alignment to use.

        @example
        element->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    */
    void SetVerticalAlignment(VerticalAlignment align) noexcept { m_verticalAlignment = align; }

    /*
        @summary
        Returns the element's position in absolute coordinates.

        @param outX
        Receives the absolute X position.

        @param outY
        Receives the absolute Y position.

        @internal
    */
    void GetAbsolutePosition(int& outX, int& outY) const noexcept;

    /*
        @summary
        Returns the element's absolute layout position, excluding render offset.

        @param outX
        Receives the absolute X position.

        @param outY
        Receives the absolute Y position.

        @internal
    */
    void GetAbsoluteLayoutPosition(int& outX, int& outY) const noexcept;

    static bool s_frameworkElementLogging;

private:
    int m_desiredWidth = 0;
    int m_desiredHeight = 0;
    int m_finalWidth = 0;
    int m_finalHeight = 0;
    int m_offsetX = 0;
    int m_offsetY = 0;
    LayoutSizeMode m_widthMode = LayoutSizeMode::Auto;
    int m_width = 0;
    int m_renderOffsetX = 0;
    int m_renderOffsetY = 0;
    LayoutSizeMode m_heightMode = LayoutSizeMode::Auto;
    int m_height = 0;
    bool m_clipToBounds = false;

protected:
    void SetRenderOffset(int x, int y) noexcept { m_renderOffsetX = x; m_renderOffsetY = y; InvalidateVisual(); }
    int GetRenderOffsetX() const noexcept { return m_renderOffsetX; }
    int GetRenderOffsetY() const noexcept { return m_renderOffsetY; }

private:
    HorizontalAlignment m_horizontalAlignment = HorizontalAlignment::Stretch;
    VerticalAlignment m_verticalAlignment = VerticalAlignment::Stretch;
};

}
