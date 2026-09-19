#include <NeoWPF/UI/Controls/ScrollViewer/ScrollViewer.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Layout/Scrolling/IScrollInfo.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <cmath>

namespace NeoWPF::UI {

ScrollViewer::ScrollViewer()
    : Content::ContentControl()
    , m_horizontalOffset(0)
    , m_verticalOffset(0)
    , m_contentWidth(0)
    , m_contentHeight(0)
    , m_viewportWidth(0)
    , m_viewportHeight(0)
    , m_horizontalScrollBarVisibility(ScrollBarVisibility::Auto)
    , m_verticalScrollBarVisibility(ScrollBarVisibility::Auto)
{
    SetHorizontalAlignment(HorizontalAlignment::Stretch);
    SetVerticalAlignment(VerticalAlignment::Stretch);
    SetClipToBounds(true);

    m_contentPresenter = std::make_shared<Content::ContentPresenter>();
    SetPresenter(m_contentPresenter);

    m_horizontalScrollBar = std::make_shared<ScrollBar>();
    m_horizontalScrollBar->SetOrientation(Orientation::Horizontal);

    m_verticalScrollBar = std::make_shared<ScrollBar>();
    m_verticalScrollBar->SetOrientation(Orientation::Vertical);

    AddVisualChild(m_contentPresenter);
    AddVisualChild(m_horizontalScrollBar);
    AddVisualChild(m_verticalScrollBar);

    InitializeScrollBars();
}

ScrollViewer::~ScrollViewer() = default;

void ScrollViewer::SetHorizontalOffset(double offset) noexcept
{
    const double clamped = std::clamp(offset, 0.0, (std::max)(0.0, m_contentWidth - m_viewportWidth));
    if (clamped == m_horizontalOffset) {
        return;
    }

    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContentElement();
        if (content) {
            auto scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
            if (scrollInfo) {
                scrollInfo->SetHorizontalOffset(clamped);
            }
        }
    }

    m_horizontalOffset = clamped;
    UpdateContentPresenterOffset();
    UpdateScrollBarValues();
    InvalidateVisual();
}

void ScrollViewer::SetVerticalOffset(double offset) noexcept
{
    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContentElement();
        if (content) {
            auto scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
            if (scrollInfo) {
                m_contentHeight = std::max(0.0, scrollInfo->GetExtentHeight());
            }
        }
    }

    const double clamped = std::clamp(offset, 0.0, (std::max)(0.0, m_contentHeight - m_viewportHeight));
    if (clamped == m_verticalOffset) {
        return;
    }

    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContentElement();
        if (content) {
            auto scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
            if (scrollInfo) {
                scrollInfo->SetVerticalOffset(clamped);
            }
        }
    }

    m_verticalOffset = clamped;
    UpdateContentPresenterOffset();
    UpdateScrollBarValues();
    InvalidateVisual();
}

void ScrollViewer::ScrollToTop() noexcept
{
    SetVerticalOffset(0);
}

void ScrollViewer::ScrollToBottom() noexcept
{
    SetVerticalOffset(std::max(0.0, m_contentHeight - m_viewportHeight));
}

void ScrollViewer::SetHorizontalScrollBarVisibility(ScrollBarVisibility visibility) noexcept
{
    if (m_horizontalScrollBarVisibility == visibility) {
        return;
    }
    m_horizontalScrollBarVisibility = visibility;
    UpdateScrollBarVisibility();
    InvalidateVisual();
}

void ScrollViewer::SetVerticalScrollBarVisibility(ScrollBarVisibility visibility) noexcept
{
    if (m_verticalScrollBarVisibility == visibility) {
        return;
    }
    m_verticalScrollBarVisibility = visibility;
    UpdateScrollBarVisibility();
    InvalidateVisual();
}

void ScrollViewer::Measure(double availableWidth, double availableHeight)
{
    const double explicitW = GetWidth();
    const double explicitH = GetHeight();
    m_viewportWidth  = explicitW > 0.0 ? explicitW  : (std::max)(0.0, availableWidth);
    m_viewportHeight = explicitH > 0.0 ? explicitH  : (std::max)(0.0, availableHeight);

    Content::ContentControl::Measure(m_viewportWidth, m_viewportHeight);

    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContentElement();
        if (content) {
            auto scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
            if (scrollInfo) {
                m_contentWidth = (std::max)(0.0, scrollInfo->GetExtentWidth());
                m_contentHeight = (std::max)(0.0, scrollInfo->GetExtentHeight());
            } else {
                m_contentWidth = m_contentPresenter->GetDesiredWidth();
                m_contentHeight = m_contentPresenter->GetDesiredHeight();
            }
        }
    }

    UpdateScrollBarState();
    UpdateScrollBarVisibility();

    SetDesiredSize(m_viewportWidth, m_viewportHeight);
}

void ScrollViewer::Arrange(double finalWidth, double finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    m_viewportWidth = (std::max)(0.0, GetFinalWidth());
    m_viewportHeight = (std::max)(0.0, GetFinalHeight());

    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContentElement();
        std::shared_ptr<Layout::Scrolling::IScrollInfo> scrollInfo;
        if (content) {
            scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
        }

        double arrangeWidth = m_viewportWidth;
        double arrangeHeight = m_viewportHeight;

        if (!scrollInfo) {
            // physical scrolling needs the full content size
            arrangeWidth = (std::max)(m_viewportWidth, m_contentPresenter->GetDesiredWidth());
            arrangeHeight = (std::max)(m_viewportHeight, m_contentPresenter->GetDesiredHeight());
        }
        m_contentPresenter->Arrange(arrangeWidth, arrangeHeight);
    }

    UpdateScrollBarState();
    UpdateContentPresenterOffset();
    UpdateScrollBarVisibility();
    UpdateOffsets();
}

void ScrollViewer::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_contentPresenter) {
        m_contentPresenter->BuildRenderCommands(commands);
    }

    if (m_horizontalScrollBar && m_horizontalScrollBar->IsVisible()) {
        m_horizontalScrollBar->BuildRenderCommands(commands);
    }

    if (m_verticalScrollBar && m_verticalScrollBar->IsVisible()) {
        m_verticalScrollBar->BuildRenderCommands(commands);
    }
}

void ScrollViewer::InitializeScrollBars() noexcept
{
    if (m_horizontalScrollBar) {
        m_horizontalScrollBar->ValueChanged += [this](double value) {
            SetHorizontalOffset(value);
        };
    }

    if (m_verticalScrollBar) {
        m_verticalScrollBar->ValueChanged += [this](double value) {
            SetVerticalOffset(value);
        };
    }
}

void ScrollViewer::UpdateScrollBarValues() noexcept
{
    if (m_horizontalScrollBar) {
        m_horizontalScrollBar->SetValue(m_horizontalOffset);
    }

    if (m_verticalScrollBar) {
        m_verticalScrollBar->SetValue(m_verticalOffset);
    }
}

void ScrollViewer::OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args)
{
    if (!args) {
        return;
    }

    const int wheelDelta = args->GetWheelDelta();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
        "[ScrollViewer] OnMouseInput wheelDelta=%d presenter=%p", wheelDelta, m_contentPresenter.get());
    if (wheelDelta == 0) {
        return;
    }

    const int delta = wheelDelta / 120;
    const int smallChange = m_verticalScrollBar ? static_cast<int>(m_verticalScrollBar->GetSmallChange()) : 16;
    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContentElement();
        if (content) {
            auto scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
            if (scrollInfo) {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
                    "[ScrollViewer] MouseWheel cast success, calling scrollInfo->MouseWheel delta=%d adjusted=%d", delta, -delta * std::max(1, smallChange));
                scrollInfo->MouseWheel(-delta * std::max(1, smallChange));
                UpdateScrollBarValues();
                InvalidateVisual();
                return;
            } else {
                NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Virtualization, NeoWPF::Diagnostics::Level::Debug,
                    "[ScrollViewer] MouseWheel cast FAILED, content=%p", content.get());
            }
        }
    }

    if (m_verticalScrollBarVisibility != ScrollBarVisibility::Disabled) {
        SetVerticalOffset(m_verticalOffset - delta * std::max(1, smallChange));
    }
}

void ScrollViewer::UpdateScrollBarState() noexcept
{
    std::shared_ptr<Layout::Scrolling::IScrollInfo> scrollInfo;
    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContentElement();
        if (content) {
            scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
        }
    }

    if (m_horizontalScrollBar) {
        const double contentSize = scrollInfo ? scrollInfo->GetExtentWidth() : m_contentWidth;
        const double viewportSize = scrollInfo ? scrollInfo->GetViewportWidth() : m_viewportWidth;
        m_horizontalScrollBar->SetContentSize(contentSize);
        m_horizontalScrollBar->SetViewportSize(viewportSize);
    }

    if (m_verticalScrollBar) {
        const double contentSize = scrollInfo ? scrollInfo->GetExtentHeight() : m_contentHeight;
        const double viewportSize = scrollInfo ? scrollInfo->GetViewportHeight() : m_viewportHeight;
        m_verticalScrollBar->SetContentSize(contentSize);
        m_verticalScrollBar->SetViewportSize(viewportSize);
    }

    UpdateScrollBarValues();
}

void ScrollViewer::UpdateScrollBarVisibility() noexcept
{
    if (m_horizontalScrollBar) {
        const bool shouldShow =
            m_horizontalScrollBarVisibility == ScrollBarVisibility::Visible ||
            (m_horizontalScrollBarVisibility == ScrollBarVisibility::Auto && m_contentWidth > m_viewportWidth);
        m_horizontalScrollBar->SetVisible(shouldShow);
    }

    if (m_verticalScrollBar) {
        const bool shouldShow =
            m_verticalScrollBarVisibility == ScrollBarVisibility::Visible ||
            (m_verticalScrollBarVisibility == ScrollBarVisibility::Auto && m_contentHeight > m_viewportHeight);
        m_verticalScrollBar->SetVisible(shouldShow);
    }
}

void ScrollViewer::UpdateOffsets() noexcept
{
    if (m_horizontalScrollBar && m_horizontalScrollBar->IsVisible()) {
        const double barHeight = 16.0;
        m_horizontalScrollBar->SetWidth(m_viewportWidth);
        m_horizontalScrollBar->SetHeight(barHeight);
        m_horizontalScrollBar->SetLayoutOffset(0.0, std::max(0.0, m_viewportHeight - barHeight));
        m_horizontalScrollBar->Arrange(m_viewportWidth, barHeight);
    }

    if (m_verticalScrollBar && m_verticalScrollBar->IsVisible()) {
        const double barWidth = 16.0;
        m_verticalScrollBar->SetWidth(barWidth);
        m_verticalScrollBar->SetHeight(m_viewportHeight);
        m_verticalScrollBar->SetLayoutOffset(std::max(0.0, m_viewportWidth - barWidth), 0.0);
        m_verticalScrollBar->Arrange(barWidth, m_viewportHeight);
    }
}

void ScrollViewer::UpdateContentPresenterOffset() noexcept
{
    if (!m_contentPresenter) {
        return;
    }

    auto content = m_contentPresenter->GetContentElement();
    if (content) {
        auto scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
        if (scrollInfo) {
            return;
        }
    }
    m_contentPresenter->SetContentRenderOffset(
        static_cast<int>(std::lround(-m_horizontalOffset)),
        static_cast<int>(std::lround(-m_verticalOffset)));
}

}
