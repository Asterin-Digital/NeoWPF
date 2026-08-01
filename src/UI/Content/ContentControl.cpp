#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/Diagnostics/Logger.h>

namespace NeoWPF::UI::Content {

bool ContentControl::s_contentControlDebugging = true;

ContentControl::ContentControl()
    : m_content(nullptr)
    , m_presenter(nullptr)
{
}

void ContentControl::SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content)
{
    if (s_contentControlDebugging) {
        NeoWPF::Diagnostics::LogInfo("[ContentControl] SetContent: content=%p presenter=%p", content.get(), m_presenter.get());
    }

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
    if (s_contentControlDebugging) {
        NeoWPF::Diagnostics::LogInfo("[ContentControl] SetPresenter: oldPresenter=%p newPresenter=%p content=%p", m_presenter.get(), presenter.get(), m_content.get());
    }

    if (m_presenter == presenter) {
        if (s_contentControlDebugging) {
            NeoWPF::Diagnostics::LogInfo("[ContentControl] SetPresenter: same presenter, skipping");
        }
        return;
    }

    if (m_presenter) {
        if (s_contentControlDebugging) {
            NeoWPF::Diagnostics::LogInfo("[ContentControl] SetPresenter: removing old presenter %p", m_presenter.get());
        }
        m_presenter->RemoveFromParent();
    }

    m_presenter = presenter;
    if (m_presenter) {
        if (s_contentControlDebugging) {
            NeoWPF::Diagnostics::LogInfo("[ContentControl] SetPresenter: using new presenter %p", m_presenter.get());
        }
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

        const int presenterWidth = m_presenter->GetDesiredWidth();
        const int presenterHeight = m_presenter->GetDesiredHeight();
        const int explicitWidth = GetWidth() > 0 ? GetWidth() : 0;
        const int explicitHeight = GetHeight() > 0 ? GetHeight() : 0;

        SetDesiredSize(
            std::max(presenterWidth, explicitWidth),
            std::max(presenterHeight, explicitHeight));
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
