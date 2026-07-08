#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>

namespace NeoWPF::UI::Content {

void ContentControl::SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content)
{
    m_content = content;
    if (m_presenter) {
        m_presenter->SetContent(content);
    }
    InvalidateVisual();
}

void ContentControl::ClearContent()
{
    m_content.reset();
    if (m_presenter) {
        m_presenter->ClearContent();
    }
    InvalidateVisual();
}

void ContentControl::SetPresenter(const std::shared_ptr<ContentPresenter>& presenter)
{
    m_presenter = presenter;
    if (m_presenter) {
        m_presenter->SetContent(m_content);
    }
    InvalidateVisual();
}

void ContentControl::Measure(int availableWidth, int availableHeight)
{
    if (m_presenter && !m_presenter->GetParent()) {
        try {
            AddChild(m_presenter);
        } catch (const std::bad_weak_ptr&) {


        }
    }

    if (m_presenter) {
        m_presenter->Measure(availableWidth, availableHeight);
        SetDesiredSize(m_presenter->GetDesiredWidth(), m_presenter->GetDesiredHeight());
        return;
    }

    FrameworkElement::Measure(availableWidth, availableHeight);
    SetDesiredSize(0, 0);
}

void ContentControl::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    if (m_presenter) {
        m_presenter->Arrange(finalWidth, finalHeight);
    }
}

void ContentControl::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_presenter) {
        m_presenter->BuildRenderCommands(commands);
    }
}

ContentBounds ContentControl::GetContentBounds() const noexcept
{
    if (m_presenter) {
        return ContentBounds{m_presenter->GetFinalWidth(), m_presenter->GetFinalHeight()};
    }

    return ContentBounds{};
}

}
