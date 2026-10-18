/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <memory>

namespace NeoWPF::UI {

/*
    @summary
    Describes how a scroll bar should be presented.

    @details
    Disabled hides the bar entirely, Hidden keeps it out of the visible layout, Visible always shows it, and Auto lets the control decide based on content size.
*/
enum class ScrollBarVisibility : int {
    Disabled = 0,
    Hidden,
    Visible,
    Auto,
};

/*
    @summary
    Hosts content and manages scrolling within a viewport.

    @details
    ScrollViewer supports both horizontal and vertical scrolling, optionally displays scroll bars, and keeps the viewport aligned with the current offset.

    @example
    auto viewer = std::make_shared<NeoWPF::UI::ScrollViewer>();
    viewer->SetHorizontalScrollBarVisibility(NeoWPF::UI::ScrollBarVisibility::Auto);
    viewer->SetVerticalScrollBarVisibility(NeoWPF::UI::ScrollBarVisibility::Visible);
*/
class ScrollViewer : public Content::ContentControl {
public:
    static const NeoWPF::UI::Properties::DependencyProperty& HorizontalOffsetProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& VerticalOffsetProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& HorizontalScrollBarVisibilityProperty();
    static const NeoWPF::UI::Properties::DependencyProperty& VerticalScrollBarVisibilityProperty();

    /*
        @summary
        Constructs a new scroll viewer.
    */
    ScrollViewer();

    /*
        @summary
        Destroys the scroll viewer.
    */
    ~ScrollViewer() override;

    /*
        @summary
        Returns the current horizontal scroll offset.

        @internal
    */
    double GetHorizontalOffset() const noexcept { return m_horizontalOffset; }

    /*
        @summary
        Returns the current vertical scroll offset.

        @internal
    */
    double GetVerticalOffset() const noexcept { return m_verticalOffset; }

    /*
        @summary
        Sets the horizontal scroll offset.

        @param offset
        The new horizontal offset in pixels.
    */
    void SetHorizontalOffset(double offset) noexcept;

    /*
        @summary
        Sets the vertical scroll offset.

        @param offset
        The new vertical offset in pixels.
    */
    void SetVerticalOffset(double offset) noexcept;

    /*
        @summary
        Scrolls the content to the top.
    */
    void ScrollToTop() noexcept;

    /*
        @summary
        Scrolls the content to the bottom.
    */
    void ScrollToBottom() noexcept;

    /*
        @summary
        Returns the current horizontal scroll bar visibility mode.

    */
    ScrollBarVisibility GetHorizontalScrollBarVisibility() const noexcept { return m_horizontalScrollBarVisibility; }

    /*
        @summary
        Returns the current vertical scroll bar visibility mode.

    */
    ScrollBarVisibility GetVerticalScrollBarVisibility() const noexcept { return m_verticalScrollBarVisibility; }

    /*
        @summary
        Sets the horizontal scroll bar visibility mode.

        @param visibility
        The new visibility mode for the horizontal scroll bar.
    */
    void SetHorizontalScrollBarVisibility(ScrollBarVisibility visibility) noexcept;

    /*
        @summary
        Sets the vertical scroll bar visibility mode.

        @param visibility
        The new visibility mode for the vertical scroll bar.
    */
    void SetVerticalScrollBarVisibility(ScrollBarVisibility visibility) noexcept;

    /*
        @summary
        Measures the scroll viewer and its content.

        @param availableWidth
        The width available for measurement.

        @param availableHeight
        The height available for measurement.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Arranges the scroll viewer and its content within the final viewport.

        @param finalWidth
        The width assigned to the control.

        @param finalHeight
        The height assigned to the control.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

    /*
        @summary
        Builds the renderer commands for the viewer and its scroll bars.

        @param commands
        The command list to populate.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    /*
        @summary
        Handles mouse input for scrolling and interaction.

        @param args
        The mouse event arguments.

        @internal
    */
    void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) override;

private:
    void InitializeScrollBars() noexcept;
    void UpdateScrollBarState() noexcept;
    void UpdateScrollBarValues() noexcept;
    void UpdateScrollBarVisibility() noexcept;
    void UpdateOffsets() noexcept;
    void UpdateContentPresenterOffset() noexcept;

    double m_horizontalOffset = 0.0;
    double m_verticalOffset = 0.0;
    double m_contentWidth = 0.0;
    double m_contentHeight = 0.0;
    double m_viewportWidth = 0.0;
    double m_viewportHeight = 0.0;

    ScrollBarVisibility m_horizontalScrollBarVisibility = ScrollBarVisibility::Auto;
    ScrollBarVisibility m_verticalScrollBarVisibility = ScrollBarVisibility::Auto;

    std::shared_ptr<ScrollBar> m_horizontalScrollBar;
    std::shared_ptr<ScrollBar> m_verticalScrollBar;

    std::shared_ptr<Content::ContentPresenter> m_contentPresenter;
};

} 
