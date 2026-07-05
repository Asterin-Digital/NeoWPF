#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include <NeoWPF/Core/Events/Event.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>

namespace NeoWPF::UI {

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
class TextBox : public FrameworkElement {
public:
    static bool s_textBoxDebug;

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

    /*
        @summary
        Returns the current text.
    */
    const std::wstring& GetText() const noexcept { return m_text; }

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
    void SetTextColor(uint32_t color) noexcept { m_textColor = color; InvalidateVisual(); }
    uint32_t GetTextColor() const noexcept { return m_textColor; }

    /*
        @summary
        Background color for the text box.

        @param color
        ARGB color value.
    */
    void SetBackgroundColor(uint32_t color) noexcept { m_backgroundColor = color; InvalidateVisual(); }
    uint32_t GetBackgroundColor() const noexcept { return m_backgroundColor; }

    /*
        @summary
        Sets the font size for text rendering.

        @param fontSize
        Size in pixels.
    */
    void SetFontSize(int fontSize) noexcept { m_fontSize = fontSize; InvalidateVisual(); }
    int GetFontSize() const noexcept { return m_fontSize; }

    /*
        @summary
        Sets padding applied around the text.

        @param padding
        Pixels of padding on all sides.
    */
    void SetPadding(int padding) noexcept { m_padding = padding; InvalidateVisual(); }
    int GetPadding() const noexcept { return m_padding; }

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
    void SetTextWrapping(bool value) noexcept { m_textWrapping = value; m_textDirty = true; InvalidateVisual(); }
    bool GetTextWrapping() const noexcept { return m_textWrapping; }

    /*
        @summary
        Border color for the control.

        @param color
        ARGB color value.
    */
    void SetBorderColor(uint32_t color) noexcept { m_borderColor = color; InvalidateVisual(); }
    uint32_t GetBorderColor() const noexcept { return m_borderColor; }

    /*
        @summary
        Caret (cursor) color.

        @param color
        ARGB color value.
    */
    void SetCaretColor(uint32_t color) noexcept { m_caretColor = color; InvalidateVisual(); }
    uint32_t GetCaretColor() const noexcept { return m_caretColor; }

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
        Keyboard input handler (text input and navigation).

        @internal
    */
    void OnKeyboardInput(NeoWPF::Input::Core::KeyboardEventArgs* args) override;

    /*
        @summary
        Called when the control receives focus.

        @internal
    */
    void OnGotFocus() override;

    /*
        @summary
        Called when the control loses focus.

        @internal
    */
    void OnLostFocus() override;

private:
    struct LineRenderData {
        uint32_t textureId = 0;
        int width = 0;
        int height = 0;
        int lineHeight = 0;
    };

    std::wstring m_text;
    std::size_t m_caretPosition = 0;
    uint32_t m_textColor = 0xFFEDEDEDu;
    uint32_t m_backgroundColor = 0xFF2F2F2Fu;
    uint32_t m_borderColor = 0xFF666666u;
    uint32_t m_caretColor = 0xFFFFFFFFu;
    int m_fontSize = 16;
    int m_padding = 4;
    bool m_isReadOnly = false;
    bool m_acceptsReturn = false;
    bool m_textWrapping = false;
    bool m_isHitTestVisible = true;

    std::vector<LineRenderData> m_lineRenderData;
    bool m_textDirty = true;
    bool m_isFocused = false;
    /*
        @summary
        Ensure GPU texture(s) backing the rendered text are present.

        @internal
    */
    void EnsureTextTexture();

    /*
        @summary
        Returns the visual X offset for the given rendered line.

        @internal
    */
    int GetLineVisualOffset(std::size_t lineIndex) const;

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
