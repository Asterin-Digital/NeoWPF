#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <utility>

#ifdef max
#undef max
#endif

namespace NeoWPF::UI::Content {



ContentPresenter::ContentPresenter()
    : m_content(nullptr)
    , m_contentElement(nullptr)
    , m_contentTemplate()
    , m_padding(0)
{
}

void ContentPresenter::SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content)
{
    SetContentData(std::any(content));
}

void ContentPresenter::SetContentData(const std::any& content)
{
    m_content = content;
    RebuildContentElement();
    InvalidateMeasure();
    InvalidateArrange();
    InvalidateVisual();
}

void ContentPresenter::SetContentTemplate(ContentTemplate contentTemplate)
{
    m_contentTemplate = std::move(contentTemplate);
    RebuildContentElement();
    InvalidateMeasure();
    InvalidateArrange();
    InvalidateVisual();
}

void ContentPresenter::RebuildContentElement()
{
    if (m_contentElement) {
        m_contentElement->RemoveFromParent();
        m_contentElement.reset();
    }

    if (!m_content.has_value()) {
        return;
    }

    if (m_contentTemplate) {
        m_contentElement = m_contentTemplate(m_content);
        return;
    }

    try {
        m_contentElement = std::any_cast<std::shared_ptr<NeoWPF::UI::UIElement>>(m_content);
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ContentPresenter,
            NeoWPF::Diagnostics::Level::Debug,
            "[ContentPresenter] Resolved content to UIElement=%p",
            m_contentElement.get());
        return;
    } catch (const std::bad_any_cast&) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ContentPresenter,
            NeoWPF::Diagnostics::Level::Debug,
            "[ContentPresenter] Content has no UIElement representation type=%s",
            m_content.type().name());
    }

}

void ContentPresenter::ClearContent()
{
    if (m_contentElement) {
        if (auto existingChild = std::dynamic_pointer_cast<Visual>(m_contentElement)) {
            existingChild->RemoveFromParent();
        }
    }
    m_content.reset();
    m_contentElement.reset();
    InvalidateMeasure();
    InvalidateArrange();
    InvalidateVisual();
}

void ContentPresenter::Measure(int availableWidth, int availableHeight)
{
    if (!m_contentElement) {
        RebuildContentElement();
    }

    if (m_contentElement && !m_contentElement->GetParent()) {
        try {
            auto visualContent = std::dynamic_pointer_cast<Visual>(m_contentElement);
            if (visualContent) {
                AddChild(visualContent);
            }
        } catch (const std::bad_weak_ptr&) {

        }
    }

    const int horizontalPadding = m_padding.Left + m_padding.Right;
    const int verticalPadding = m_padding.Top + m_padding.Bottom;
    int desiredWidth = horizontalPadding;
    int desiredHeight = verticalPadding;

    if (m_contentElement) {
        const int contentAvailableWidth = availableWidth > 0 ? std::max(0, availableWidth - horizontalPadding) : 0;
        const int contentAvailableHeight = availableHeight > 0 ? std::max(0, availableHeight - verticalPadding) : 0;
        m_contentElement->Measure(contentAvailableWidth, contentAvailableHeight);
        if (auto* frameworkContent = dynamic_cast<FrameworkElement*>(m_contentElement.get())) {
            desiredWidth = std::max(desiredWidth, frameworkContent->GetDesiredWidth() + horizontalPadding);
            desiredHeight = std::max(desiredHeight, frameworkContent->GetDesiredHeight() + verticalPadding);
        }
    }

    SetDesiredSize(desiredWidth, desiredHeight);
}

void ContentPresenter::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ContentPresenter, NeoWPF::Diagnostics::Level::Debug,
        "[ContentPresenter] Arrange: this=%p final=(%d,%d) padding=(%d,%d,%d,%d)", this, finalWidth, finalHeight,
        m_padding.Left, m_padding.Top, m_padding.Right, m_padding.Bottom);

    if (m_contentElement) {
        const int contentWidth = std::max(0, finalWidth - (m_padding.Left + m_padding.Right));
        const int contentHeight = std::max(0, finalHeight - (m_padding.Top + m_padding.Bottom));

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ContentPresenter, NeoWPF::Diagnostics::Level::Debug,
            "[ContentPresenter] Arrange: contentArea=(%d,%d)", contentWidth, contentHeight);

        if (auto* childFe = dynamic_cast<FrameworkElement*>(m_contentElement.get())) {
            childFe->Arrange(contentWidth, contentHeight);

            int offsetX = 0;
            int offsetY = 0;
            const int actualW = std::max(0, childFe->GetFinalWidth());
            const int actualH = std::max(0, childFe->GetFinalHeight());

            switch (childFe->GetHorizontalAlignment()) {
            case HorizontalAlignment::Left:
                offsetX = 0;
                break;
            case HorizontalAlignment::Center:
                offsetX = (contentWidth - actualW) / 2;
                break;
            case HorizontalAlignment::Right:
                offsetX = contentWidth - actualW;
                break;
            case HorizontalAlignment::Stretch:
                offsetX = (contentWidth - actualW) / 2;
                break;
            }

            switch (childFe->GetVerticalAlignment()) {
            case VerticalAlignment::Top:
                offsetY = 0;
                break;
            case VerticalAlignment::Center:
                offsetY = (contentHeight - actualH) / 2;
                break;
            case VerticalAlignment::Bottom:
                offsetY = contentHeight - actualH;
                break;
            case VerticalAlignment::Stretch:
                offsetY = (contentHeight - actualH) / 2;
                break;
            }

            childFe->SetLayoutOffset(offsetX + m_padding.Left, offsetY + m_padding.Top);

            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::ContentPresenter, NeoWPF::Diagnostics::Level::Debug,
                "[ContentPresenter] Arrange: child=%p actual=(%d,%d) offset=(%d,%d) layoutOffset=(%d,%d)",
                childFe, actualW, actualH, offsetX, offsetY, childFe->GetLayoutOffsetX(), childFe->GetLayoutOffsetY());
        } else {
            m_contentElement->Arrange(contentWidth, contentHeight);
        }
    }
}

void ContentPresenter::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!m_contentElement) {
        return;
    }

    if (m_contentElement->IsVisible()) {
        m_contentElement->BuildRenderCommands(commands);
    }
}

void ContentPresenter::SetContentRenderOffset(int x, int y) noexcept
{
    SetRenderOffset(x, y);
}

ContentBounds ContentPresenter::GetContentBounds() const noexcept
{
    return ContentBounds{GetFinalWidth(), GetFinalHeight()};
}

}
