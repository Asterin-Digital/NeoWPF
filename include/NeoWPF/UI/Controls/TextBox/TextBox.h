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
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Controls/Templates/ControlTemplate.h>
#include <NeoWPF/UI/Controls/ScrollBar/ScrollBar.h>
#include <NeoWPF/UI/Styling/State/VisualStateManager.h>

namespace NeoWPF::UI {

class UIElement;
void ApplyTextBoxThemeStyle(UIElement* element);

/*
    @summary
    Controls how text wraps inside the available width

    @details
    `NoWrap` keeps the text on a single line, `Wrap` breaks lines when space runs out, and
    `WrapWithOverflow` wraps while still allowing the content to extend past the bounds.
*/
enum class TextWrapping {
    NoWrap,
    Wrap,
    WrapWithOverflow
};

class TextBoxStyle;  
void RegisterTextBoxStyles();

class TextBoxTemplate; 

class TextLayout {
public:
    struct LineRenderData {
        std::wstring text;
        uint32_t textureId = 0;
        int width = 0;
        int height = 0;
        int lineHeight = 0;
        std::size_t charStart = 0;
        std::size_t charEnd = 0;
    };

    TextLayout() = default;
    ~TextLayout();

    void Refresh(const std::wstring& text, int fontSize, int availableWidth, int availableHeight, TextWrapping textWrapping, bool createTextures = true);
    void Clear();

    const std::vector<LineRenderData>& GetLines() const noexcept { return m_lines; }
    std::size_t GetLineCount() const noexcept { return m_lines.size(); }
    int GetContentWidth() const noexcept { return m_contentWidth; }
    int GetContentHeight() const noexcept { return m_contentHeight; }

    int GetLineHeight(std::size_t lineIndex) const;
    int GetLineVisualOffset(std::size_t lineIndex) const;
    std::pair<std::size_t, std::size_t> GetCaretLineColumn(std::size_t caretPosition) const;
    int GetCaretY(std::size_t caretPosition) const;
    int GetCaretHeight(std::size_t caretPosition) const;
    static int MeasureLineHeight(const std::wstring& text, int fontSize);

private:
    std::wstring m_text;
    int m_fontSize = 16;
    std::vector<LineRenderData> m_lines;
    int m_contentWidth = 0;
    int m_contentHeight = 0;
    int m_availableWidth = 0;
    int m_availableHeight = 0;
    TextWrapping m_textWrapping = TextWrapping::NoWrap;
};

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
        @summary
        Sets padding applied around the text.

        @param padding
        Pixels of padding on all sides.
    */
    void SetPadding(int padding) noexcept;
    int GetPadding() const noexcept;

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
    void OnPaddingChanged(int padding) override;
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
    std::wstring m_text;
    std::size_t m_caretPosition = 0;
    NeoWPF::UI::Color m_textColor = NeoWPF::UI::Color::FromArgb(0xFFEDEDEDu);
    NeoWPF::UI::Color m_caretColor = NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu);
    int m_fontSize = 16;
    bool m_isReadOnly = false;
    bool m_acceptsReturn = false;
    TextWrapping m_textWrapping = TextWrapping::NoWrap;
    bool m_isHitTestVisible = true;

    std::shared_ptr<Styling::VisualStateManager> m_visualStateManager;
    std::string m_currentVisualState;

    TextLayout m_textLayout;
    bool m_textDirty = true;
    bool m_isFocused = false;
    std::shared_ptr<ScrollBar> m_verticalScrollBar;
    std::shared_ptr<ScrollBar> m_horizontalScrollBar;
    bool m_isDraggingVerticalScrollbar = false;
    bool m_isDraggingHorizontalScrollbar = false;
    int m_lastDragPointer = 0;
    int m_dragStartScrollValueX = 0;
    int m_dragStartScrollValueY = 0;
    int m_dragStartPointerX = 0;
    int m_dragStartPointerY = 0;
    int m_viewportWidth = 0;
    int m_viewportHeight = 0;
    int m_contentHostWidth = 0;
    int m_contentHostHeight = 0;
    bool m_showVerticalScrollbar = false;
    bool m_showHorizontalScrollbar = false;
    /*
        @summary
        Ensure GPU texture(s) backing the rendered text are present.

        @internal
    */
    void EnsureTextTexture();

    /*
        @summary
        Updates the current content size and scrollbar visibility.

        @internal
    */
    void UpdateScrollMetrics();

    /*
        @summary
        Keeps the current scroll offsets inside the valid range.

        @internal
    */
    void ClampScrollOffsets();

    /*
        @summary
        Makes sure the current caret is visible within the viewport.

        @internal
    */
    void EnsureCaretVisible();

    /*
        @summary
        Scrolls the text viewport to the requested content offsets.

        @internal
    */
    void ScrollTo(int x, int y);

    /*
        @summary
        Returns the visible width available for text content.

        @internal
    */
    int GetViewportWidth() const;

    /*
        @summary
        Returns the visible height available for text content.

        @internal
    */
    int GetViewportHeight() const;

    /*
        @summary
        Returns the maximum horizontal scroll offset.

        @internal
    */
    int GetMaxHorizontalScroll() const;

    /*
        @summary
        Returns the maximum vertical scroll offset.

        @internal
    */
    int GetMaxVerticalScroll() const;

    /*
        @summary
        Returns the vertical thumb length in pixels.

        @internal
    */
    int GetVerticalThumbLength() const;

    /*
        @summary
        Returns the vertical thumb position in pixels.

        @internal
    */
    int GetVerticalThumbPosition() const;

    /*
        @summary
        Returns the horizontal thumb length in pixels.

        @internal
    */
    int GetHorizontalThumbPosition() const;

    /*
        @summary
        Returns the horizontal thumb length in pixels.

        @internal
    */
    int GetHorizontalThumbLength() const;

    /*
        @summary
        Returns the visual Y offset for the given rendered line.

        @internal
    */
    int GetLineVisualOffset(std::size_t lineIndex) const;

    /*
        @summary
        Scrolls the viewport based on a pointer position inside the vertical bar track.

        @internal
    */
    void UpdateVerticalScrollFromPointer(int pointerY);

    /*
        @summary
        Scrolls the viewport based on a pointer position inside the horizontal bar track.

        @internal
    */
    void UpdateHorizontalScrollFromPointer(int pointerX);

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
