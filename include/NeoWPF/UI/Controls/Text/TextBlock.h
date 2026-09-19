/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <cstdint>
#include <string>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Element/IContentAware.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>
#include <NeoWPF/UI/Layout/Placement/TextWrapping.h>
#include <NeoWPF/UI/Text/TextRasterCache.h>
#include <NeoWPF/UI/Text/FontStyle.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>

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
    static const Properties::DependencyProperty& TextProperty();
    static const Properties::DependencyProperty& TextColorProperty();
    static const Properties::DependencyProperty& FontSizeProperty();
    static const Properties::DependencyProperty& FontFamilyProperty();
    static const Properties::DependencyProperty& FontWeightProperty();
    static const Properties::DependencyProperty& FontStyleProperty();
    static const Properties::DependencyProperty& TextHorizontalAlignmentProperty();
    static const Properties::DependencyProperty& TextVerticalAlignmentProperty();
    static const Properties::DependencyProperty& TextWrappingProperty();
    static const Properties::DependencyProperty& PaddingProperty();

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
        Attaches a live localization expression to the text property.

        @param key
        The application-owned localization key to resolve.

        @details
        The text updates when localization changes. Calling SetText replaces the locale value.
    */
    void SetLocale(const std::wstring& key);

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
        Sets the font weight used to measure and render text.
    */
    void SetFontWeight(int fontWeight) noexcept;

    /*
        @summary
        Returns the current font weight used to measure and render text.
    */
    int GetFontWeight() const noexcept;

    /*
        @summary
        Sets the font style used to measure and render text.
    */
    void SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept;

    /*
        @summary
        Returns the current font style used to measure and render text.
    */
    NeoWPF::UI::FontStyle GetFontStyle() const noexcept;

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
        Sets text wrapping behavior.

        @param wrapping
        The wrapping mode to apply (NoWrap, Wrap, WrapWithOverflow).
    */
    void SetTextWrapping(TextWrapping wrapping) noexcept;

    /*
        @summary
        Returns the current text wrapping mode.
    */
    TextWrapping GetTextWrapping() const noexcept;

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
    Thickness GetPadding() const noexcept;

    /*
        @summary
        Sets the interior padding in device pixels.

        @param padding
        Padding applied to each side.
    */
    void SetPadding(Thickness padding) noexcept;

    /*
        @summary
        Measures the text and reports the desired size including padding.

        @param availableWidth
        The width available to the element.

        @param availableHeight
        The height available to the element.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Arranges the text inside the final bounds and prepares rendering resources.

        @param finalWidth
        The width assigned to the element.

        @param finalHeight
        The height assigned to the element.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

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
    ContentBounds GetContentBounds() const noexcept override {
        return { m_textCache.GetContentWidth(), m_textCache.GetContentHeight() };
    }

    /*
        @summary
        Returns the measured content width (excluding padding).

        @internal
    */
    double GetContentWidth() const noexcept { return m_textCache.GetContentWidth(); }

    /*
        @summary
        Returns the measured content height (excluding padding).

        @internal
    */
    double GetContentHeight() const noexcept { return m_textCache.GetContentHeight(); }

protected:
    void OnPaddingChanged(Thickness padding);

private:
    std::wstring m_text;
    NeoWPF::UI::Color m_textColor = NeoWPF::UI::Color::FromArgb(0xFF000000u);
    int m_fontSize = 16;
    std::wstring m_fontFamily = std::wstring(L"Segoe UI");
    int m_fontWeight = 400;
    NeoWPF::UI::FontStyle m_fontStyle = NeoWPF::UI::FontStyle::Normal;
    double m_padding = 0.0;
    TextHorizontalAlignment m_textHorizontalAlignment = TextHorizontalAlignment::Left;
    TextVerticalAlignment m_textVerticalAlignment = TextVerticalAlignment::Top;
    TextWrapping m_textWrapping = TextWrapping::NoWrap;
    bool m_textDirty = true;
    bool m_isHitTestVisible = true;
    int m_lastArrangeWidth = -1;

    NeoWPF::UI::Text::TextRasterCache m_textCache;

    void EnsureTextTexture(int availableWidth, int availableHeight);
};

}
