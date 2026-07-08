#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <memory>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Content {

class ContentPresenter : public FrameworkElement, public IContentAware {
public:
    ContentPresenter() = default;

    void SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content);
    const std::shared_ptr<NeoWPF::UI::UIElement>& GetContent() const noexcept { return m_content; }
    void ClearContent();

    int GetPadding() const noexcept { return m_padding; }
    void SetPadding(int padding) noexcept {
        m_padding = padding;
        InvalidateVisual();
    }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    bool IsHitTestVisible() const noexcept override { return false; }
    ContentBounds GetContentBounds() const noexcept override;

private:
    std::shared_ptr<NeoWPF::UI::UIElement> m_content;
    int m_padding = 0;
};

}
