#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Input/Core/Events.h>
#include <algorithm>
#include <cmath>

#ifdef max
#undef max
#endif

namespace {
    uint32_t GetCaretTextureId() {
        static uint32_t s_caretTextureId = 0;
        if (s_caretTextureId == 0) {
            const uint32_t whitePixel = 0xFFFFFFFFu;
            s_caretTextureId = NeoWPF::Rendering::Renderer_CreateTexture(1, 1, &whitePixel);
        }
        return s_caretTextureId;
    }
}

#ifdef min
#undef min
#endif

bool NeoWPF::UI::TextBox::s_textBoxDebug = false;

namespace NeoWPF::UI {

TextBox::TextBox()
    : FrameworkElement()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetClipToBounds(true);

    if (auto style = Styling::StyleManager::GetDefault(typeid(TextBox))) {
        style->Apply(this);
    }

    InitializeVisualStates();
    ApplyVisualState("Normal", false);
}

TextBox::~TextBox()
{
    for (auto& line : m_lineRenderData) {
        if (line.textureId != 0) {
            NeoWPF::Rendering::Renderer_DestroyTexture(line.textureId);
            line.textureId = 0;
        }
    }
    m_lineRenderData.clear();
}

void TextBox::SetText(const std::wstring& text)
{
    if (m_text == text) {
        return;
    }

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] SetText: oldLength=%zu, newLength=%zu", m_text.size(), text.size());
    }

    m_text = text;
    m_caretPosition = std::min(m_caretPosition, m_text.size());
    m_scrollBar.Reset();
    m_textDirty = true;
    InvalidateVisual();
    TextChanged.Invoke(m_text);
}

void TextBox::Measure(int availableWidth, int availableHeight)
{
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    const int minWidth = 120;
    const int minHeight = std::max(baseMetrics.height, m_fontSize) + (m_padding * 2);
    const int desiredWidth = std::max(minWidth, availableWidth > 0 ? std::min(minWidth, availableWidth) : minWidth);
    const int desiredHeight = std::max(minHeight, availableHeight > 0 ? std::min(minHeight, availableHeight) : minHeight);

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] Measure: available=%dx%d, desired=%dx%d, textLength=%zu", availableWidth, availableHeight, desiredWidth, desiredHeight, m_text.size());
    }

    SetDesiredSize(desiredWidth, desiredHeight);
}

void TextBox::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    UpdateScrollMetrics();
    ClampScrollOffsets();
    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] Arrange: final=%dx%d, textDirty=%d, lineCount=%zu", finalWidth, finalHeight, m_textDirty ? 1 : 0, m_lineRenderData.size());
    }
    if (m_textDirty) {
        EnsureTextTexture();
    }
    EnsureCaretVisible();
}



void TextBox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX, absY;
    GetAbsolutePosition(absX, absY);

    auto* border = dynamic_cast<Templates::TemplateVisual*>(FindTemplateChild("PART_Border"));
    if (border) {
        border->SetRect(absX, absY, GetFinalWidth(), GetFinalHeight());
        border->SetOpacity(GetOpacity());
        border->BuildRenderCommands(commands);
    }
    else {
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            static_cast<float>(GetFinalWidth()),
            static_cast<float>(GetFinalHeight()),
            m_backgroundColor,
            GetOpacity());

        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            static_cast<float>(GetFinalWidth()),
            1.0f,
            m_borderColor,
            GetOpacity());
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY + GetFinalHeight() - 1),
            static_cast<float>(GetFinalWidth()),
            1.0f,
            m_borderColor,
            GetOpacity());
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            1.0f,
            static_cast<float>(GetFinalHeight()),
            m_borderColor,
            GetOpacity());
        commands.DrawRect(
            static_cast<float>(absX + GetFinalWidth() - 1),
            static_cast<float>(absY),
            1.0f,
            static_cast<float>(GetFinalHeight()),
            m_borderColor,
            GetOpacity());
    }

    auto* contentHost = dynamic_cast<Templates::TemplateVisual*>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const int inset = std::max(0, m_padding);
        contentHost->SetRect(absX + inset, absY + inset, std::max(0, GetFinalWidth() - (inset * 2)), std::max(0, GetFinalHeight() - (inset * 2)));
        contentHost->SetFillColor(0x00000000u);
        contentHost->SetBorderColor(0x00000000u);
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHost->BuildRenderCommands(commands);
    }

    const int scrollbarThickness = 12;
    const int contentInset = m_padding;
    const int viewportWidth = GetViewportWidth();
    const int viewportHeight = GetViewportHeight();
    const int contentX = absX + contentInset;
    const int contentY = absY + contentInset;
    const auto clip = NeoWPF::Rendering::ClipRect{ static_cast<float>(contentX), static_cast<float>(contentY), static_cast<float>(viewportWidth), static_cast<float>(viewportHeight), true };

    int currentY = -m_scrollBar.GetScrollOffsetY();
    for (const auto& line : m_lineRenderData) {
        if (line.textureId == 0 || line.width <= 0 || line.height <= 0) {
            currentY += std::max(1, line.lineHeight);
            continue;
        }

        const int drawWidth = std::min(line.width, viewportWidth);
        const float uvRight = drawWidth > 0 ? static_cast<float>(drawWidth) / static_cast<float>(line.width) : 1.0f;
        const int drawHeight = std::max(1, line.height);
        const float uvBottom = drawHeight > 0 ? static_cast<float>(drawHeight) / static_cast<float>(line.height) : 1.0f;
        const int lineVerticalOffset = GetLineVisualOffset(static_cast<std::size_t>(&line - &m_lineRenderData[0]));

        if (s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] BuildRenderCommands: line=%zu width=%d height=%d drawWidth=%d drawHeight=%d uvRight=%.3f uvBottom=%.3f", m_lineRenderData.size() > 0 ? static_cast<std::size_t>(&line - &m_lineRenderData[0]) : 0, line.width, line.height, drawWidth, drawHeight, uvRight, uvBottom);
        }

        commands.DrawTexturedQuad(
            static_cast<float>(contentX - m_scrollBar.GetScrollOffsetX()),
            static_cast<float>(contentY + currentY + lineVerticalOffset),
            static_cast<float>(drawWidth),
            static_cast<float>(drawHeight),
            0.0f, 0.0f,
            uvRight, uvBottom,
            line.textureId,
            GetOpacity(),
            clip);

        currentY += std::max(1, line.lineHeight);
    }

    if (m_isFocused) {
        int caretX = GetCaretX();
        int rawCaretX = caretX;
        int caretY = GetCaretY();
        caretX = std::min(caretX, viewportWidth);
        const int caretHeight = std::max(1, GetCaretHeight());
        const auto [caretLineIndex, _] = GetCaretLineColumn(m_caretPosition);
        const int caretVerticalOffset = GetLineVisualOffset(caretLineIndex);
        if (s_textBoxDebug) {
            NeoWPF::Diagnostics::LogInfo("[TextBox] BuildRenderCommands: caretPosition=%zu rawCaretX=%d clampedCaretX=%d caretY=%d caretVerticalOffset=%d visibleTextWidth=%d", m_caretPosition, rawCaretX, caretX, caretY, caretVerticalOffset, viewportWidth);
        }

        const uint32_t caretTextureId = GetCaretTextureId();
        commands.DrawTexturedQuad(
            static_cast<float>(contentX + caretX - m_scrollBar.GetScrollOffsetX()),
            static_cast<float>(contentY + caretY + caretVerticalOffset - m_scrollBar.GetScrollOffsetY()),
            1.0f,
            static_cast<float>(caretHeight),
            0.0f, 0.0f,
            1.0f, 1.0f,
            caretTextureId,
            GetOpacity(),
            clip);
    }

    if (m_scrollBar.HasVerticalScrollbar()) {
        const int barX = absX + GetFinalWidth() - scrollbarThickness;
        const int barY = absY + contentInset;
        const int barHeight = std::max(1, GetViewportHeight());
        const int thumbLength = m_scrollBar.GetVerticalThumbLength();
        const int thumbPosition = m_scrollBar.GetVerticalThumbPosition();
        commands.DrawRect(static_cast<float>(barX), static_cast<float>(barY), static_cast<float>(scrollbarThickness), static_cast<float>(barHeight), 0xFF3A3A3Au, GetOpacity());
        commands.DrawRect(static_cast<float>(barX), static_cast<float>(barY + thumbPosition), static_cast<float>(scrollbarThickness), static_cast<float>(thumbLength), 0xFF7A7A7Au, GetOpacity());
    }

    if (m_scrollBar.HasHorizontalScrollbar()) {
        const int barX = absX + contentInset;
        const int barY = absY + GetFinalHeight() - scrollbarThickness;
        const int barWidth = std::max(1, GetViewportWidth());
        const int thumbLength = m_scrollBar.GetHorizontalThumbLength();
        const int thumbPosition = m_scrollBar.GetHorizontalThumbPosition();
        commands.DrawRect(static_cast<float>(barX), static_cast<float>(barY), static_cast<float>(barWidth), static_cast<float>(scrollbarThickness), 0xFF3A3A3Au, GetOpacity());
        commands.DrawRect(static_cast<float>(barX + thumbPosition), static_cast<float>(barY), static_cast<float>(thumbLength), static_cast<float>(scrollbarThickness), 0xFF7A7A7Au, GetOpacity());
    }
}

void TextBox::UpdateScrollMetrics()
{
    int contentWidth = 0;
    int contentHeight = 0;

    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    std::size_t lineStart = 0;
    while (lineStart <= m_text.size()) {
        const std::size_t lineBreak = m_text.find(L'\n', lineStart);
        const std::size_t lineEnd = lineBreak == std::wstring::npos ? m_text.size() : lineBreak;
        const std::wstring lineText = m_text.substr(lineStart, lineEnd - lineStart);
        const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(lineText.empty() ? L"W" : lineText, m_fontSize, 0, 0);
        const int lineHeight = std::max(baseMetrics.height, std::max(lineMetrics.height, m_fontSize));
        contentHeight += lineHeight;
        contentWidth = std::max(contentWidth, lineMetrics.width);
        if (lineBreak == std::wstring::npos) {
            break;
        }
        lineStart = lineEnd + 1;
    }

    m_scrollBar.UpdateMetrics(contentWidth, contentHeight, GetFinalWidth(), GetFinalHeight(), m_padding);
    ClampScrollOffsets();
}

void TextBox::ClampScrollOffsets()
{
    m_scrollBar.ClampScrollOffsets();
}

void TextBox::EnsureCaretVisible()
{
    if (m_text.empty()) {
        return;
    }

    const auto [lineIndex, _] = GetCaretLineColumn(m_caretPosition);
    int caretY = 0;
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    std::size_t lineStart = 0;
    std::size_t currentLine = 0;
    while (lineStart < m_text.size() && currentLine < lineIndex) {
        const std::size_t nextNewline = m_text.find(L'\n', lineStart);
        const std::size_t lineEnd = nextNewline == std::wstring::npos ? m_text.size() : nextNewline;
        const std::wstring lineText = m_text.substr(lineStart, lineEnd - lineStart);
        const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(lineText.empty() ? L"W" : lineText, m_fontSize, 0, 0);
        caretY += std::max(baseMetrics.height, std::max(lineMetrics.height, m_fontSize));
        lineStart = lineEnd + 1;
        ++currentLine;
    }

    const int viewportHeight = GetViewportHeight();
    const int caretHeight = std::max(1, GetCaretHeight());
    if (caretY < m_scrollBar.GetScrollOffsetY()) {
        m_scrollBar.SetScrollOffset(m_scrollBar.GetScrollOffsetX(), caretY);
    }
    else if (caretY + caretHeight > m_scrollBar.GetScrollOffsetY() + viewportHeight) {
        m_scrollBar.SetScrollOffset(m_scrollBar.GetScrollOffsetX(), std::max(0, caretY + caretHeight - viewportHeight));
    }

    const int caretX = std::max(0, GetCaretX());
    const int viewportWidth = GetViewportWidth();
    if (caretX < m_scrollBar.GetScrollOffsetX()) {
        m_scrollBar.SetScrollOffset(caretX, m_scrollBar.GetScrollOffsetY());
    }
    else if (caretX > m_scrollBar.GetScrollOffsetX() + viewportWidth) {
        m_scrollBar.SetScrollOffset(std::max(0, caretX - viewportWidth + 8), m_scrollBar.GetScrollOffsetY());
    }

    ClampScrollOffsets();
}

void TextBox::ScrollTo(int x, int y)
{
    m_scrollBar.ScrollTo(x, y);
    InvalidateVisual();
}

int TextBox::GetViewportWidth() const
{
    return m_scrollBar.GetViewportWidth();
}

int TextBox::GetViewportHeight() const
{
    return m_scrollBar.GetViewportHeight();
}

int TextBox::GetMaxHorizontalScroll() const
{
    return m_scrollBar.GetMaxHorizontalScroll();
}

int TextBox::GetMaxVerticalScroll() const
{
    return m_scrollBar.GetMaxVerticalScroll();
}

int TextBox::GetVerticalThumbLength() const
{
    return m_scrollBar.GetVerticalThumbLength();
}

int TextBox::GetVerticalThumbPosition() const
{
    return m_scrollBar.GetVerticalThumbPosition();
}

int TextBox::GetHorizontalThumbLength() const
{
    return m_scrollBar.GetHorizontalThumbLength();
}

int TextBox::GetHorizontalThumbPosition() const
{
    return m_scrollBar.GetHorizontalThumbPosition();
}

void TextBox::UpdateVerticalScrollFromPointer(int pointerY)
{
    m_scrollBar.UpdateVerticalScrollFromPointer(pointerY);
    InvalidateVisual();
}

void TextBox::UpdateHorizontalScrollFromPointer(int pointerX)
{
    m_scrollBar.UpdateHorizontalScrollFromPointer(pointerX);
    InvalidateVisual();
}

int TextBox::GetLineVisualOffset(std::size_t lineIndex) const
{
    if (lineIndex < m_lineRenderData.size()) {
        const auto& line = m_lineRenderData[lineIndex];
        if (line.height > 0 && line.lineHeight > line.height) {
            return (line.lineHeight - line.height) / 2;
        }
    }
    return 0;
}

void TextBox::EnsureTextTexture()
{
    if (!m_textDirty) {
        return;
    }

    if (s_textBoxDebug) {
        NeoWPF::Diagnostics::LogInfo("[TextBox] EnsureTextTexture: textLength=%zu, lineCount=%zu", m_text.size(), m_lineRenderData.size());
    }

    for (auto& line : m_lineRenderData) {
        if (line.textureId != 0) {
            NeoWPF::Rendering::Renderer_DestroyTexture(line.textureId);
            line.textureId = 0;
        }
    }
    m_lineRenderData.clear();

    if (m_text.empty()) {
        m_textDirty = false;
        return;
    }

    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, 0, 0);
    std::size_t lineStart = 0;
    while (lineStart <= m_text.size()) {
        const std::size_t lineEnd = m_text.find(L'\n', lineStart);
        const std::size_t nextLine = lineEnd == std::wstring::npos ? m_text.size() : lineEnd;
        const std::wstring lineText = m_text.substr(lineStart, nextLine - lineStart);

        LineRenderData lineData;
        if (!lineText.empty()) {
            int width = 0;
            int height = 0;
            auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(lineText, m_fontSize, width, height);
            if (s_textBoxDebug) {
                NeoWPF::Diagnostics::LogInfo("[TextBox] RenderTextBitmap result: line='%ls' width=%d height=%d bitmapSize=%zu", lineText.c_str(), width, height, bitmap.size());
            }
            if (!bitmap.empty() && width > 0 && height > 0) {
                lineData.textureId = NeoWPF::Rendering::Renderer_CreateTexture(static_cast<unsigned int>(width), static_cast<unsigned int>(height), bitmap.data());
                lineData.width = width;
                lineData.height = height;
            }
        }

        const auto lineMetrics = NeoWPF::Rendering::Text::MeasureText(lineText.empty() ? L"W" : lineText, m_fontSize, 0, 0);
        if (lineData.height > 0) {
            lineData.lineHeight = lineData.height;
        }
        else {
            lineData.lineHeight = std::max(baseMetrics.height, std::max(lineMetrics.height, m_fontSize));
        }
        m_lineRenderData.push_back(lineData);

        if (lineEnd == std::wstring::npos) {
            break;
        }
        lineStart = lineEnd + 1;
    }

    m_textDirty = false;
}

}
