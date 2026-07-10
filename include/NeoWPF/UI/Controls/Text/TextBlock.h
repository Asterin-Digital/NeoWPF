#pragma once

#include <string>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::Input::Core
{
    class MouseEventArgs;
    class KeyboardEventArgs;
}

namespace NeoWPF::UI::Styling {
    class Style;
}

namespace NeoWPF::UI {

class TextBlockStyle;
void RegisterTextBlockStyles();

class TextBlock : public FrameworkElement, public IContentAware {
public:
    TextBlock();
    explicit TextBlock(const std::wstring& text);
    ~TextBlock();

    void SetText(const std::wstring& text);
    const std::wstring& GetText() const noexcept { return m_text; }

    void SetTextColor(uint32_t color) noexcept { m_textColor = color; InvalidateVisual(); }
    uint32_t GetTextColor() const noexcept { return m_textColor; }

    void SetFontSize(int fontSize) noexcept { m_fontSize = fontSize; InvalidateVisual(); }
    int GetFontSize() const noexcept { return m_fontSize; }

    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    bool IsHitTestVisible() const noexcept override { return m_isHitTestVisible; }
    void SetHitTestVisible(bool visible) noexcept { m_isHitTestVisible = visible; }

    ContentBounds GetContentBounds() const noexcept override { return {m_textWidth, m_textHeight}; }

    int GetContentWidth() const noexcept { return m_textWidth; }
    int GetContentHeight() const noexcept { return m_textHeight; }

private:
    std::wstring m_text;
    uint32_t m_textColor = 0xFF000000u;
    int m_fontSize = 16;
    uint32_t m_textureId = 0;
    int m_textWidth = 0;
    int m_textHeight = 0;
    bool m_textDirty = true;
    bool m_isHitTestVisible = true;
    int m_lastArrangeWidth = 0;
    int m_lastArrangeHeight = 0;

    void EnsureTextTexture(int availableWidth, int availableHeight);
};

}
