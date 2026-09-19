/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <any>
#include <memory>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Content {

class ContentPresenter;

/*
    @summary
    Hosts content through a content presenter.
*/
class ContentControl : public NeoWPF::UI::Control, public NeoWPF::UI::IContentAware {
public:
    static const Properties::DependencyProperty& ContentProperty();

    ContentControl();
    ~ContentControl() override;

    /*
        @summary
        Sets a visual as the content.

        @param content
        Content visual.
    */
    void SetContent(const std::shared_ptr<NeoWPF::UI::UIElement>& content);

    /*
        @summary
        Sets content data for the presenter to display.

        @param content
        Content value.
    */
    void SetContentData(const std::any& content);
    const std::any& GetContent() const noexcept { return m_content; }
    std::shared_ptr<NeoWPF::UI::UIElement> GetContentElement() const noexcept;
    /*
        @summary
        Removes the current content.
    */
    void ClearContent();

    /*
        @summary
        Sets the presenter used to display the content.

        @param presenter
        Content presenter.
    */
    void SetPresenter(const std::shared_ptr<ContentPresenter>& presenter);
    const std::shared_ptr<ContentPresenter>& GetPresenter() const noexcept { return m_presenter; }

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    ContentBounds GetContentBounds() const noexcept override;

private:
    std::any m_content;
    std::shared_ptr<ContentPresenter> m_presenter;
};

}
