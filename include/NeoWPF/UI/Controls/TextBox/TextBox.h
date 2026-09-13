/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/UI/Content/ContentControl.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>
#include <NeoWPF/UI/Layout/Placement/TextWrapping.h>
#include <NeoWPF/UI/Text/TextRasterCache.h>
#include <NeoWPF/UI/Text/FontStyle.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

namespace NeoWPF::UI {

class UIElement;

enum class CaretScrollBehavior {
    Disabled,
    Enabled,
    EnabledForSingleLineNoWrap
};

class TextBoxStyle;  
void RegisterTextBoxStyles();

class TextBoxTemplate; 

/*
    @summary
    A single-line (or wrapping) text input control.

    @details
    `TextBox` renders editable text, manages a caret, and fires `TextChanged` when the content
    changes. It supports read-only mode, basic wrapping, and appearance options (colors, font
    size, padding). Use this control for simple text input inside layouts.

    @example
    auto tb = std::make_shared<NeoWPF::UI::TextBox>();
    tb->SetText(L"Hello world");
    tb->SetFontSize(14);
    tb->SetPadding(6);
    // register a handler function: avoid inlined block comments inside the example
    tb->TextChanged += OnTextChanged;
*/
class TextBox : public NeoWPF::UI::Content::ContentControl {
public:
    static bool s_textBoxDebug;
    static const Properties::DependencyProperty& TextProperty();
    static const Properties::DependencyProperty& PlaceholderTextProperty();
    static const Properties::DependencyProperty& TextColorProperty();
    static const Properties::DependencyProperty& CaretColorProperty();
    static const Properties::DependencyProperty& FontSizeProperty();
    static const Properties::DependencyProperty& FontFamilyProperty();
    static const Properties::DependencyProperty& FontWeightProperty();
    static const Properties::DependencyProperty& FontStyleProperty();
    static const Properties::DependencyProperty& TextHorizontalAlignmentProperty();
    static const Properties::DependencyProperty& TextVerticalAlignmentProperty();

    /*
        @summary
        Creates a new TextBox with sensible defaults.
    */
    TextBox();

    /*
        @summary
        Destroys the TextBox and frees any backing render resources.
    */
    ~TextBox();

    /*
        @summary
        Replaces the current text.

        @param text
        New text to display.

        @example
        tb->SetText(L"Paste this in");
    */
    void SetText(const std::wstring& text);
    void SetText(const std::wstring& text, bool resetScrollOffset);

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
        Sets the caret-following policy for this TextBox.

        @internal
        @param value
        The desired caret-following mode.
    */
    void SetCaretScroll(CaretScrollBehavior value) noexcept;

    /*
        @summary
        Returns the current caret-following policy.

        @internal
    */
    CaretScrollBehavior GetCaretScroll() const noexcept { return m_caretScrollBehavior; }

    /*
        @summary
        Enables or disables caret-following using a simple boolean switch.

        @internal
        @param value
        `true` to enable the feature, `false` to disable it.
    */
    void SetCaretFollow(bool value) noexcept { SetCaretScroll(value ? CaretScrollBehavior::EnabledForSingleLineNoWrap : CaretScrollBehavior::Disabled); }

    /*
        @summary
        Returns whether caret-following is currently active.

        @internal
    */
    bool IsCaretFollow() const noexcept { return m_caretScrollBehavior != CaretScrollBehavior::Disabled; }

    /*
        @summary
        Returns the current text.
    */
    const std::wstring& GetText() const noexcept { return m_text; }

    /*
        @summary
        Sets a ghost-text placeholder shown while the box is empty.

        @param placeholder
        The text to render when the box has no real content.
    */
    void SetPlaceholderText(const std::wstring& placeholder);
    const std::wstring& GetPlaceholderText() const noexcept;

    /*
        @summary
        Raised when the text changes.

        @details
        Handlers receive the new text value. This fires for programmatic and user edits.
    */
    NeoWPF::Core::Event<const std::wstring&> TextChanged;

    /*
        @summary
        Put the box into read-only mode.

        @param value
        `true` to disable editing, `false` to allow edits.

        @example
        tb->SetReadOnly(true); // useful for read-only editors
    */
    void SetReadOnly(bool value) noexcept { m_isReadOnly = value; InvalidateVisual(); }
    bool IsReadOnly() const noexcept { return m_isReadOnly; }

    /*
        @summary
        Text color used for rendering.

        @param color
        ARGB color value.
    */
    void SetTextColor(NeoWPF::UI::Color color) noexcept;
    void SetTextColor(std::uint32_t color) noexcept;
    NeoWPF::UI::Color GetTextColor() const noexcept;

    /*
        @summary
        Background color for the text box.

        @param color
        ARGB color value.
    */
    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept;
    void SetBackgroundColor(std::uint32_t color) noexcept;
    NeoWPF::UI::Color GetBackgroundColor() const noexcept;

    /*
        @summary
        Sets the font size for text rendering.

        @param fontSize
        Size in pixels.
    */
    void SetFontSize(int fontSize) noexcept;
    int GetFontSize() const noexcept;

    /*
        @summary        Sets the font family used to measure and render text.

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
        Returns the current font family.
    */
    std::wstring GetFontFamily() const noexcept;

    /*
        @summary
        Sets the horizontal alignment used when placing text inside the TextBox.

        @param alignment
        The horizontal text alignment to apply.
    */
    void SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept;

    /*
        @summary
        Returns the current horizontal alignment used for text inside the TextBox.
    */
    TextHorizontalAlignment GetTextHorizontalAlignment() const noexcept;

    /*
        @summary
        Sets the vertical alignment used when placing text inside the TextBox.

        @param alignment
        The vertical text alignment to apply.
    */
    void SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept;

    /*
        @summary
        Returns the current vertical alignment used for text inside the TextBox.
    */
    TextVerticalAlignment GetTextVerticalAlignment() const noexcept;

    /*
        @summary        Sets padding applied around the text.

        @param padding
        Pixels of padding on all sides.
    */
    void SetPadding(Thickness padding) noexcept;
    Thickness GetPadding() const noexcept;

    /*
        @summary
        Controls whether the control accepts newline input.

        @param value
        `true` to accept returns; `false` to ignore them.

        @example
        tb->SetAcceptsReturn(true); // multiline input when used inside a taller container
    */
    void SetAcceptsReturn(bool value) noexcept { m_acceptsReturn = value; InvalidateVisual(); }
    bool GetAcceptsReturn() const noexcept { return m_acceptsReturn; }

    /*
        @summary
        Enables basic text wrapping.

        @param value
        `true` to wrap long lines, `false` to keep a single line.
    */
    void SetTextWrapping(TextWrapping value) noexcept { m_textWrapping = value; m_textDirty = true; InvalidateVisual(); }
    TextWrapping GetTextWrappingMode() const noexcept { return m_textWrapping; }
    void SetTextWrapping(bool value) noexcept { SetTextWrapping(value ? TextWrapping::Wrap : TextWrapping::NoWrap); }
    bool GetTextWrapping() const noexcept { return GetTextWrappingMode() != TextWrapping::NoWrap; }

    /*
        @summary
        Border color for the control.

        @param color
        ARGB color value.
    */
    void SetBorderColor(NeoWPF::UI::Color color) noexcept;
    void SetBorderColor(std::uint32_t color) noexcept;
    NeoWPF::UI::Color GetBorderColor() const noexcept;

    /*
        @summary
        Caret (cursor) color.

        @param color
        ARGB color value.
    */
    void SetCaretColor(NeoWPF::UI::Color color) noexcept;
    void SetCaretColor(std::uint32_t color) noexcept;
    NeoWPF::UI::Color GetCaretColor() const noexcept;

    /*
        @summary
        Measures desired size for layout.

        @internal
    */
    void Measure(int availableWidth, int availableHeight) override;

    /*
        @summary
        Arranges the control into its final bounds.

        @internal
    */
    void Arrange(int finalWidth, int finalHeight) override;

protected:
    void OnInitialize() override;
    void OnPaddingChanged(Thickness padding) override;
    void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                     const Properties::PropertyMetadata& metadata,
                                     const std::any& oldValue,
                                     const std::any& newValue,
                                     Properties::PropertyValueSource source) override;
    void OnBackgroundColorChanged(NeoWPF::UI::Color color) override;
    void OnBorderColorChanged(NeoWPF::UI::Color color) override;
    void OnBorderThicknessChanged(int thickness) override;

    /*
        @summary
        Produces rendering commands for the frame.

        @internal
    */
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    /*
        @summary
        Controls whether the TextBox participates in hit testing.

        @details
        By default a TextBox is hit-test visible so it receives mouse events. Call
        `SetHitTestVisible(false)` to opt out and allow events to pass through.
    */
    bool IsHitTestVisible() const noexcept override { return m_isHitTestVisible; }

    /*
        @summary
        Enable or disable hit testing for this control.

        @param value
        `true` to receive mouse events (default), `false` to ignore them.
    */
    void SetHitTestVisible(bool value) noexcept { m_isHitTestVisible = value; }
    bool IsInteractable() const noexcept override { return IsEnabled(); }

    /*
        @summary
        Mouse-down handler (used for caret placement and selection).

        @internal
    */
    void OnMouseDown(int x, int y, std::uint32_t buttonState) override;

    /*
        @summary
        Ends any active scrollbar drag interaction when is released

        @internal
    */
    void OnMouseUp(int x, int y, std::uint32_t buttonState) override;

    /*
        @summary
        Handles mouse movement and wheel input for scrolling when the text overflows.

        @internal
    */
    void OnMouseInput(NeoWPF::Input::Core::MouseEventArgs* args) override;

    /*
        @summary
        Keyboard input handler (text input and navigation).

        @internal
    */
    void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) override;

    /*
        @summary
        Initialize visual state groups for the TextBox.

        @internal
    */
    void InitializeVisualStates();

    /*
        @summary
        Apply a visual state to the TextBox.

        @internal
    */
    void ApplyVisualState(const std::string& stateName, bool useTransitions = false);

private:
    struct CaretState {
        std::size_t position = 0;
        bool visible = false;
    };


    std::wstring m_text;
    std::wstring m_placeholderText;
    NeoWPF::UI::Color m_textColor = NeoWPF::UI::Color::FromArgb(0xFFEDEDEDu);
    NeoWPF::UI::Color m_caretColor = NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu);
    int m_fontSize = 16;
    TextHorizontalAlignment m_textHorizontalAlignment = TextHorizontalAlignment::Left;
    TextVerticalAlignment m_textVerticalAlignment = TextVerticalAlignment::Top;
    std::wstring m_fontFamily = std::wstring(L"Segoe UI");
    int m_fontWeight = 400;
    NeoWPF::UI::FontStyle m_fontStyle = NeoWPF::UI::FontStyle::Normal;
    bool m_isReadOnly = false;
    bool m_acceptsReturn = false;
    TextWrapping m_textWrapping = TextWrapping::NoWrap;
    CaretScrollBehavior m_caretScrollBehavior = CaretScrollBehavior::EnabledForSingleLineNoWrap;
    bool m_isHitTestVisible = true;
    bool m_isUpdatingText = false;

    std::shared_ptr<Styling::VisualStateManager> m_visualStateManager;
    std::string m_currentVisualState;

    NeoWPF::UI::Text::TextRasterCache m_renderCache;
    bool m_textDirty = true;
    bool m_isFocused = false;
    CaretState m_caret;
    int m_scrollOffsetX = 0;
    int m_scrollOffsetY = 0;

    /*
        @summary
        Ensure GPU texture(s) backing the rendered text are present.

        @internal
    */
    void EnsureTextTexture();

    /*
        @summary
        Returns the visual Y offset for the given rendered line.

        @internal
    */
    int GetLineVisualOffset(std::size_t lineIndex) const;

    /*
        @summary
        Ensures the caret stays visible when there is scrolling.

        @internal
    */
    void EnsureCaretVisible();

    int ClampHorizontalScrollOffset(int offset) const noexcept;
    int ClampVerticalScrollOffset(int offset) const noexcept;
    void SetScrollOffset(int x, int y) noexcept;

    /*
        @summary
        Inserts a character at the current caret position.

        @internal
    */
    void InsertCharacter(wchar_t character);

    /*
        @summary
        Removes a character (backspace/delete behavior).

        @internal
    */
    void RemoveCharacter(bool backspace);

    /*
        @summary
        Updates caret position based on mouse coordinates.

        @internal
    */
    void UpdateCaretFromMouse(int x, int y);

    /*
        @summary
        Computes the caret X position in pixels.

        @internal
    */
    int GetCaretX() const;

    /*
        @summary
        Computes the caret Y position in pixels.

        @internal
    */
    int GetCaretY() const;

    /*
        @summary
        Returns the caret height used for rendering.

        @internal
    */
    int GetCaretHeight() const;

    /*
        @summary
        Converts a flat caret position into line/column pair.

        @internal
    */
    std::pair<std::size_t, std::size_t> GetCaretLineColumn(std::size_t caretPosition) const;

    /*
        @summary
        Move caret to a specific line and column.

        @internal
    */
    void MoveCaretToLineColumn(std::size_t lineIndex, std::size_t column);

    /*
        @summary
        Returns the measured height for a given line string.

        @internal
    */
    int GetLineHeight(const std::wstring& line) const;
};

class TextBoxTemplate : public Templates::ControlTemplate {
public:
    TextBoxTemplate();
};

}
