#pragma once

#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <memory>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Content {

class ContentPresenter;

class ContentControl : public NeoWPF::UI::Control, public NeoWPF::UI::IContentAware {
public:
    static const Properties::DependencyProperty& ContentProperty();

    ContentControl();

    void SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content);
    const std::shared_ptr<NeoWPF::UI::UIElement>& GetContent() const noexcept { return m_content; }
    void ClearContent();

    void SetPresenter(const std::shared_ptr<ContentPresenter>& presenter);
    const std::shared_ptr<ContentPresenter>& GetPresenter() const noexcept { return m_presenter; }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    ContentBounds GetContentBounds() const noexcept override;

private:
    std::shared_ptr<NeoWPF::UI::UIElement> m_content;
    std::shared_ptr<ContentPresenter> m_presenter;
};

}
