#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>
#include <memory>

namespace NeoWPF::UI {

class Border : public NeoWPF::UI::Content::ContentControl {
public:
    Border();

    using ContentControl::SetContent;
    using ContentControl::GetContent;
    using ContentControl::ClearContent;

    void SetChild(const std::shared_ptr<NeoWPF::UI::UIElement>& child) { SetContent(child); }
    const std::shared_ptr<NeoWPF::UI::UIElement>& GetChild() const noexcept { return GetContent(); }
    void ClearChild() { ClearContent(); }

    void SetPadding(int padding) noexcept { m_padding = padding; InvalidateVisual(); }
    int GetPadding() const noexcept { return m_padding; }

    void SetBorderThickness(int thickness) noexcept { m_borderThickness = thickness; InvalidateVisual(); }
    int GetBorderThickness() const noexcept { return m_borderThickness; }

    void SetBorderColor(uint32_t color) noexcept { m_borderColor = color; InvalidateVisual(); }
    uint32_t GetBorderColor() const noexcept { return m_borderColor; }

    void SetBackgroundColor(uint32_t color) noexcept { m_backgroundColor = color; InvalidateVisual(); }
    uint32_t GetBackgroundColor() const noexcept { return m_backgroundColor; }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

private:
    int m_padding = 0;
    int m_borderThickness = 0;
    uint32_t m_borderColor = 0xFF000000u;
    uint32_t m_backgroundColor = 0x00000000u;
};

}
