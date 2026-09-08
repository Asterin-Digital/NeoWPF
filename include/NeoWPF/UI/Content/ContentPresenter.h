#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <any>
#include <functional>
#include <memory>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Content {

class ContentPresenter : public FrameworkElement, public IContentAware {
public:
    using ContentTemplate = std::function<std::shared_ptr<NeoWPF::UI::UIElement>(const std::any&)>;

    ContentPresenter();

    void SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content);
    void SetContentData(const std::any& content);
    const std::any& GetContent() const noexcept { return m_content; }
    std::shared_ptr<NeoWPF::UI::UIElement> GetContentElement() const noexcept { return m_contentElement; }
    void ClearContent();

    void SetContentTemplate(ContentTemplate contentTemplate);
    const ContentTemplate& GetContentTemplate() const noexcept { return m_contentTemplate; }

    Thickness GetPadding() const noexcept { return m_padding; }
    void SetPadding(Thickness padding) noexcept {
        m_padding = padding;
        InvalidateMeasure();
        InvalidateArrange();
        InvalidateVisual();
    }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    bool IsHitTestVisible() const noexcept override { return false; }
    ContentBounds GetContentBounds() const noexcept override;

    void SetContentRenderOffset(int x, int y) noexcept;

private:
    void RebuildContentElement();

    std::any m_content;
    std::shared_ptr<NeoWPF::UI::UIElement> m_contentElement;
    ContentTemplate m_contentTemplate;
    Thickness m_padding{};
};

}
