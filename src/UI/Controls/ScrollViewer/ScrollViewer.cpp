#include <NeoWPF/UI/Controls/ScrollViewer/ScrollViewer.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Layout/Scrolling/IScrollInfo.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

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

void ScrollViewer::SetHorizontalOffset(int offset) noexcept
{
    const int clamped = std::clamp(offset, 0, std::max(0, m_contentWidth - m_viewportWidth));
    if (clamped == m_horizontalOffset) {
        return;
    }

    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContent();
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

void ScrollViewer::SetVerticalOffset(int offset) noexcept
{
    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContent();
        if (content) {
            auto scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
            if (scrollInfo) {
                m_contentHeight = std::max(0, scrollInfo->GetExtentHeight());
            }
        }
    }

    const int clamped = std::clamp(offset, 0, std::max(0, m_contentHeight - m_viewportHeight));
    if (clamped == m_verticalOffset) {
        return;
    }

    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContent();
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
    SetVerticalOffset(std::max(0, m_contentHeight - m_viewportHeight));
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

void ScrollViewer::Measure(int availableWidth, int availableHeight)
{
    m_viewportWidth = std::max(0, availableWidth);
    m_viewportHeight = std::max(0, availableHeight);

    Content::ContentControl::Measure(m_viewportWidth, m_viewportHeight);

    if (m_contentPresenter) {
        auto content = m_contentPresenter->GetContent();
        if (content) {
            auto scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
            if (scrollInfo) {
                m_contentWidth = std::max(0, scrollInfo->GetExtentWidth());
                m_contentHeight = std::max(0, scrollInfo->GetExtentHeight());
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

void ScrollViewer::Arrange(int finalWidth, int finalHeight)
{
    const int width = std::max(0, finalWidth);
    const int height = std::max(0, finalHeight);

    m_viewportWidth = width;
    m_viewportHeight = height;

    Content::ContentControl::Arrange(width, height);

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
        m_horizontalScrollBar->ValueChanged += [this](int value) {
            SetHorizontalOffset(value);
        };
    }

    if (m_verticalScrollBar) {
        m_verticalScrollBar->ValueChanged += [this](int value) {
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
        auto content = m_contentPresenter->GetContent();
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
        auto content = m_contentPresenter->GetContent();
        if (content) {
            scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
        }
    }

    if (m_horizontalScrollBar) {
        const int contentSize = scrollInfo ? scrollInfo->GetExtentWidth() : m_contentWidth;
        const int viewportSize = scrollInfo ? scrollInfo->GetViewportWidth() : m_viewportWidth;
        m_horizontalScrollBar->SetContentSize(contentSize);
        m_horizontalScrollBar->SetViewportSize(viewportSize);
    }

    if (m_verticalScrollBar) {
        const int contentSize = scrollInfo ? scrollInfo->GetExtentHeight() : m_contentHeight;
        const int viewportSize = scrollInfo ? scrollInfo->GetViewportHeight() : m_viewportHeight;
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
        const int barHeight = 16;
        m_horizontalScrollBar->SetWidth(m_viewportWidth);
        m_horizontalScrollBar->SetHeight(barHeight);
        m_horizontalScrollBar->SetLayoutOffset(0, std::max(0, m_viewportHeight - barHeight));
        m_horizontalScrollBar->Arrange(m_viewportWidth, barHeight);
    }

    if (m_verticalScrollBar && m_verticalScrollBar->IsVisible()) {
        const int barWidth = 16;
        m_verticalScrollBar->SetWidth(barWidth);
        m_verticalScrollBar->SetHeight(m_viewportHeight);
        m_verticalScrollBar->SetLayoutOffset(std::max(0, m_viewportWidth - barWidth), 0);
        m_verticalScrollBar->Arrange(barWidth, m_viewportHeight);
    }
}

void ScrollViewer::UpdateContentPresenterOffset() noexcept
{
    if (!m_contentPresenter) {
        return;
    }

    auto content = m_contentPresenter->GetContent();
    if (content) {
        auto scrollInfo = std::dynamic_pointer_cast<Layout::Scrolling::IScrollInfo>(content);
        if (scrollInfo) {
            return;
        }
    }
    m_contentPresenter->SetContentRenderOffset(-m_horizontalOffset, -m_verticalOffset);
}

}
