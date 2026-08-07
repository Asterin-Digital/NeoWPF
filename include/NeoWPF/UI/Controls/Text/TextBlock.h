#pragma once

#include <cstdint>
#include <string>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>
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

/*
    @summary
    Displays read-only text inside layouts.

    @details
    `TextBlock` is a element for showing text

    @example
    auto tb = std::make_shared<NeoWPF::UI::TextBlock>(L"Hello world");
    tb->SetFontSize(18);
    tb->SetPadding(8);
*/
class TextBlock : public FrameworkElement, public IContentAware {
public:
    TextBlock();
    explicit TextBlock(const std::wstring& text);
    ~TextBlock();

    /*
        @summary
        Sets the text content.

        @param text
        The text to display.
    */
    void SetText(const std::wstring& text);

    /*
        @summary
        Returns the current text content.
    */
    const std::wstring& GetText() const noexcept { return m_text; }


    /*
        @summary
        Sets the text color used when rendering.

        @param color
        The color to use.
    */
    void SetTextColor(NeoWPF::UI::Color color) noexcept { m_textColor = color; InvalidateVisual(); }
    void SetTextColor(std::uint32_t color) noexcept { SetTextColor(NeoWPF::UI::Color::FromArgb(color)); }

    /*
        @summary
        Returns the current text color.
    */
    NeoWPF::UI::Color GetTextColor() const noexcept { return m_textColor; }

    /*
        @summary
        Sets the font size used to measure and render text.

        @param fontSize
        Font size in device pixels.
    */
    void SetFontSize(int fontSize) noexcept { m_fontSize = fontSize; InvalidateVisual(); }

    /*
        @summary
        Returns the current font size.
    */
    int GetFontSize() const noexcept { return m_fontSize; }

    /*
        @summary
        Sets the font family used to measure and render text.

        @param fontFamily
        The font family name, for example `L"Segoe UI"`.
    */
    void SetFontFamily(const std::wstring& fontFamily) noexcept;

    /*
        @summary
        Sets the horizontal alignment used when placing text inside the TextBlock.

        @param alignment
        The horizontal text alignment to apply.
    */
    void SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept;

    /*
        @summary
        Returns the current horizontal alignment used for text inside the TextBlock.
    */
    TextHorizontalAlignment GetTextHorizontalAlignment() const noexcept;

    /*
        @summary
        Sets the vertical alignment used when placing text inside the TextBlock.

        @param alignment
        The vertical text alignment to apply.
    */
    void SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept;

    /*
        @summary
        Returns the current vertical alignment used for text inside the TextBlock.
    */
    TextVerticalAlignment GetTextVerticalAlignment() const noexcept;

    /*
        @summary
        Returns the current font family.
    */
    std::wstring GetFontFamily() const noexcept;

    /*
        @summary
        Gets the padding inside the `TextBlock` bounds.

        @details
        Padding is interior spacing applied between the `TextBlock`'s bounds and its rendered text.
    */
    int GetPadding() const noexcept;

    /*
        @summary
        Sets the interior padding in device pixels.

        @param padding
        Uniform padding applied to all sides.
    */
    void SetPadding(int padding) noexcept;

    /*
        @summary
        Measures the text and reports the desired size including padding.

        @param availableWidth
        The width available to the element.

        @param availableHeight
        The height available to the element.

        @internal
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Arranges the text inside the final bounds and prepares rendering resources.

        @param finalWidth
        The width assigned to the element.

        @param finalHeight
        The height assigned to the element.

        @internal
    */
    void Arrange(int finalWidth, int finalHeight) override;

    /*
        @summary
        Appends rendering commands to draw the text texture.

        @param commands
        The command list to populate.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    bool IsHitTestVisible() const noexcept override { return m_isHitTestVisible; }
    void SetHitTestVisible(bool visible) noexcept { m_isHitTestVisible = visible; }

    /*
        @summary
        Returns the content bounds (measured text size, excluding padding).

        @internal
    */
    ContentBounds GetContentBounds() const noexcept override { return {m_textWidth, m_textHeight}; }

    /*
        @summary
        Returns the measured content width (excluding padding).

        @internal
    */
    int GetContentWidth() const noexcept { return m_textWidth; }

    /*
        @summary
        Returns the measured content height (excluding padding).

        @internal
    */
    int GetContentHeight() const noexcept { return m_textHeight; }

protected:
    void OnPaddingChanged(int padding);

private:
    std::wstring m_text;
    NeoWPF::UI::Color m_textColor = NeoWPF::UI::Color::FromArgb(0xFF000000u);
    int m_fontSize = 16;
    std::wstring m_fontFamily = std::wstring(L"Segoe UI");
    int m_padding = 0;
    uint32_t m_textureId = 0;
    int m_textWidth = 0;
    int m_textHeight = 0;
    int m_textInkTop = 0;
    int m_textInkHeight = 0;
    TextHorizontalAlignment m_textHorizontalAlignment = TextHorizontalAlignment::Left;
    TextVerticalAlignment m_textVerticalAlignment = TextVerticalAlignment::Top;
    bool m_textDirty = true;
    bool m_isHitTestVisible = true;
    int m_lastArrangeWidth = 0;
    int m_lastArrangeHeight = 0;

    void EnsureTextTexture(int availableWidth, int availableHeight);
};

}
