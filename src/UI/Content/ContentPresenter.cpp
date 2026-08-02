#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace NeoWPF::UI::Content {

bool ContentPresenter::s_contentPresenterDebugging = false;

ContentPresenter::ContentPresenter()
    : m_content(nullptr)
    , m_padding(0)
{
}

#ifdef max
#undef max
#endif

void ContentPresenter::SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content)
{
    if (s_contentPresenterDebugging) {
        NeoWPF::Diagnostics::LogInfo("[ContentPresenter] SetContent: oldContent=%p newContent=%p", m_content.get(), content.get());
    }

    if (m_content && m_content != content) {
        if (auto existingChild = std::dynamic_pointer_cast<Visual>(m_content)) {
            if (s_contentPresenterDebugging) {
                NeoWPF::Diagnostics::LogInfo("[ContentPresenter] SetContent: removing old content %p from parent %p", existingChild.get(), existingChild->GetParent());
            }
            existingChild->RemoveFromParent();
        }
    }

    if (content) {
        if (auto visualContent = std::dynamic_pointer_cast<Visual>(content)) {
            if (s_contentPresenterDebugging) {
                NeoWPF::Diagnostics::LogInfo("[ContentPresenter] SetContent: removing new content %p from existing parent %p", visualContent.get(), visualContent->GetParent());
            }
            visualContent->RemoveFromParent();
        }
    }

    m_content = content;
    InvalidateVisual();
}

void ContentPresenter::ClearContent()
{
    if (s_contentPresenterDebugging) {
        NeoWPF::Diagnostics::LogInfo("[ContentPresenter] ClearContent: content=%p", m_content.get());
    }

    if (m_content) {
        if (auto existingChild = std::dynamic_pointer_cast<Visual>(m_content)) {
            existingChild->RemoveFromParent();
        }
        m_content.reset();
    }
    InvalidateVisual();
}

void ContentPresenter::Measure(int availableWidth, int availableHeight)
{
    if (m_content && !m_content->GetParent()) {
        try {
            auto visualContent = std::dynamic_pointer_cast<Visual>(m_content);
            if (visualContent) {
                AddChild(visualContent);
            }
        } catch (const std::bad_weak_ptr&) {

        }
    }

    int desiredWidth = m_padding * 2;
    int desiredHeight = m_padding * 2;

    if (m_content) {
        m_content->Measure(availableWidth, availableHeight);
        if (auto* frameworkContent = dynamic_cast<FrameworkElement*>(m_content.get())) {
            desiredWidth = std::max(desiredWidth, frameworkContent->GetDesiredWidth() + (m_padding * 2));
            desiredHeight = std::max(desiredHeight, frameworkContent->GetDesiredHeight() + (m_padding * 2));
        }
    }

    SetDesiredSize(desiredWidth, desiredHeight);
}

void ContentPresenter::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    if (m_content) {
        const int contentWidth = std::max(0, finalWidth);
        const int contentHeight = std::max(0, finalHeight);

        if (auto* childFe = dynamic_cast<FrameworkElement*>(m_content.get())) {
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

            childFe->SetLayoutOffset(offsetX, offsetY);
        } else {
            m_content->Arrange(contentWidth, contentHeight);
        }
    }
}

void ContentPresenter::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!m_content) {
        return;
    }

    if (m_content->IsVisible()) {
        m_content->BuildRenderCommands(commands);
    }
}

ContentBounds ContentPresenter::GetContentBounds() const noexcept
{
    return ContentBounds{GetFinalWidth(), GetFinalHeight()};
}

}
