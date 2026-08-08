#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <memory>

namespace NeoWPF::UI {

enum class ScrollBarVisibility : int {
    Disabled = 0,
    Hidden,
    Visible,
    Auto,
};

class ScrollViewer : public Content::ContentControl {
public:
    ScrollViewer();
    ~ScrollViewer() override;

    int GetHorizontalOffset() const noexcept { return m_horizontalOffset; }
    int GetVerticalOffset() const noexcept { return m_verticalOffset; }

    void SetHorizontalOffset(int offset) noexcept;
    void SetVerticalOffset(int offset) noexcept;

    ScrollBarVisibility GetHorizontalScrollBarVisibility() const noexcept { return m_horizontalScrollBarVisibility; }
    ScrollBarVisibility GetVerticalScrollBarVisibility() const noexcept { return m_verticalScrollBarVisibility; }

    void SetHorizontalScrollBarVisibility(ScrollBarVisibility visibility) noexcept;
    void SetVerticalScrollBarVisibility(ScrollBarVisibility visibility) noexcept;

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;
    void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) override;

private:
    void UpdateScrollBarState() noexcept;
    void UpdateScrollBarVisibility() noexcept;
    void UpdateOffsets() noexcept;
    void EnsureScrollBarTemplates() noexcept;
    void UpdateContentPresenterOffset() noexcept;

    int m_horizontalOffset = 0;
    int m_verticalOffset = 0;
    int m_contentWidth = 0;
    int m_contentHeight = 0;
    int m_viewportWidth = 0;
    int m_viewportHeight = 0;

    ScrollBarVisibility m_horizontalScrollBarVisibility = ScrollBarVisibility::Auto;
    ScrollBarVisibility m_verticalScrollBarVisibility = ScrollBarVisibility::Auto;

    std::shared_ptr<ScrollBar> m_horizontalScrollBar;
    std::shared_ptr<ScrollBar> m_verticalScrollBar;

    std::shared_ptr<Content::ContentPresenter> m_contentPresenter;
};

} // namespace NeoWPF::UI
