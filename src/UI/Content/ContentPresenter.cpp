#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>

#ifdef max
#undef max
#endif

namespace NeoWPF::UI::Content {

void ContentPresenter::SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content)
{
    if (m_content && m_content != content) {
        if (auto existingChild = std::dynamic_pointer_cast<Visual>(m_content)) {
            if (existingChild->GetParent() == this) {
                RemoveVisualChild(existingChild);
            }
        }
    }

    m_content = content;
    InvalidateVisual();
}

void ContentPresenter::ClearContent()
{
    if (m_content) {
        if (auto existingChild = std::dynamic_pointer_cast<Visual>(m_content)) {
            if (existingChild->GetParent() == this) {
                RemoveVisualChild(existingChild);
            }
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
        m_content->Measure(availableWidth - (m_padding * 2), availableHeight - (m_padding * 2));
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
        const int contentWidth = std::max(0, finalWidth - (m_padding * 2));
        const int contentHeight = std::max(0, finalHeight - (m_padding * 2));
        m_content->Arrange(contentWidth, contentHeight);
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
    ContentBounds bounds{};
    if (auto* contentAware = dynamic_cast<IContentAware*>(m_content.get())) {
        bounds = contentAware->GetContentBounds();
    } else if (auto* frameworkContent = dynamic_cast<FrameworkElement*>(m_content.get())) {
        bounds.width = frameworkContent->GetDesiredWidth();
        bounds.height = frameworkContent->GetDesiredHeight();
    }
    return bounds;
}

}
