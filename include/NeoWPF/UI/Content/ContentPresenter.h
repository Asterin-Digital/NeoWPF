/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <any>
#include <functional>
#include <memory>
#include <type_traits>

namespace NeoWPF::UI {
class UIElement;
}

namespace NeoWPF::UI::Content {

/*
    @summary
    Presents visual or data content inside a layout.
*/
class ContentPresenter : public FrameworkElement, public IContentAware {
public:
    /*
        @summary
        Creates a visual from the supplied content value.
    */
    using ContentTemplate = std::function<std::shared_ptr<NeoWPF::UI::UIElement>(const std::any&)>;

    ContentPresenter();

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
    /*
        @summary
        Sets a UI element as content.

        @param content
        Content visual.
    */
    template <typename T, std::enable_if_t<std::is_base_of_v<NeoWPF::UI::UIElement, T>, int> = 0>
    void SetContentData(const std::shared_ptr<T>& content)
    {
        SetContentData(std::any(std::static_pointer_cast<NeoWPF::UI::UIElement>(content)));
    }

    /*
        @summary
        Returns the current content value supplied to the presenter.
    */
    const std::any& GetContent() const noexcept { return m_content; }
    /*
        @summary
        Returns the current content when it is a UI element.
    */
    std::shared_ptr<NeoWPF::UI::UIElement> GetContentElement() const noexcept { return m_contentElement; }
    /*
        @summary
        Removes the current content.
    */
    void ClearContent();

    /*
        @summary
        Sets the factory used to create content visuals.

        @param contentTemplate
        Content factory.
    */
    void SetContentTemplate(ContentTemplate contentTemplate);

    /*
        @summary
        Returns the factory used to create content visuals.
    */
    const ContentTemplate& GetContentTemplate() const noexcept { return m_contentTemplate; }

    /*
        @summary
        Returns the padding around the presenter content.
    */
    Thickness GetPadding() const noexcept { return m_padding; }
    /*
        @summary
        Sets the presenter padding.

        @param padding
        Padding around the content.
    */
    void SetPadding(Thickness padding) noexcept {
        m_padding = padding;
        InvalidateMeasure();
        InvalidateArrange();
        InvalidateVisual();
    }

    /*
        @summary
        Measures the presented content using the available layout space.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;
    /*
        @summary
        Arranges the presented content within the final layout size.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;
    /*
        @summary
        Adds the presented content's drawing operations to a command list.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    /*
        @summary
        Reports whether the presenter can receive hit tests.

        @internal
    */
    bool IsHitTestVisible() const noexcept override { return false; }
    /*
        @summary
        Returns the bounds occupied by the presented content.
    */
    ContentBounds GetContentBounds() const noexcept override;

    /*
        @summary
        Sets the content render offset.

        @param x
        Horizontal offset.

        @param y
        Vertical offset.
    */
    void SetContentRenderOffset(int x, int y) noexcept;

private:
    void RebuildContentElement();

    std::any m_content;
    std::shared_ptr<NeoWPF::UI::UIElement> m_contentElement;
    ContentTemplate m_contentTemplate;
    Thickness m_padding{};
};

}
