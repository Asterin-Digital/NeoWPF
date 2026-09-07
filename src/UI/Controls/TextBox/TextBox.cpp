#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/TextBoxStyles.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/UI/Layout/Placement/TextAlignment.h>
#include <NeoWPF/Input/Core/Events.h>
#include <algorithm>
#include <cmath>
#include <mutex>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
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

    void ReleaseCaretTextureId() {
        const uint32_t caretTextureId = GetCaretTextureId();
        if (caretTextureId != 0) {
            NeoWPF::Rendering::Renderer_DestroyTexture(caretTextureId);
        }
    }
}

namespace NeoWPF::UI {

TextBox::TextBox()
    : Content::ContentControl()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetClipToBounds(true);
    SetTemplate(std::make_shared<TextBoxTemplate>());

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(GetPadding());
    SetPresenter(presenter);

    NeoWPF::UI::Generated::TextBoxStyles styles;
    styles.InitializeComponent();
    NEOWPF_APPLY_STYLES(styles, *this);

    InitializeVisualStates();
}

void TextBox::OnInitialize()
{
    UIElement::OnInitialize();
    ApplyVisualState("Normal", false);
}

void TextBox::SetCaretScroll(CaretScrollBehavior value) noexcept
{
    m_caretScrollBehavior = value;
    InvalidateVisual();
}

TextBox::~TextBox()
{
    ReleaseCaretTextureId();
}

void TextBox::InitializeVisualStates()
{
    m_visualStateManager = std::make_shared<Styling::VisualStateManager>();
}

void TextBox::ApplyVisualState(const std::string& stateName, bool useTransitions)
{
    if (stateName.empty()) {
        return;
    }

    m_currentVisualState = stateName;

    if (auto templateManager = GetTemplateVisualStateManager()) {
        templateManager->GoToState(this, stateName, useTransitions);
    }
    else if (m_visualStateManager) {
        m_visualStateManager->GoToState(this, stateName, useTransitions);
    }
}

void TextBox::SetText(const std::wstring& text)
{
    SetText(text, true);
}

void TextBox::SetText(const std::wstring& text, bool resetScrollOffset)
{
    if (m_text == text) {
        return;
    }

    if (m_isUpdatingText) {
        m_text = text;
        m_caret.position = std::min(m_caret.position, m_text.size());
        m_textDirty = true;
        if (resetScrollOffset) {
            if (GetFinalWidth() > 0 && GetFinalHeight() > 0) {
                SetScrollOffset(0, 0);
            }
        }
        InvalidateVisual();
        TextChanged.Invoke(m_text);
        return;
    }

    m_isUpdatingText = true;

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Text", *this);
    if (property && !IsApplyingPropertyValue()) {
        SetValue(*property, text);
        m_isUpdatingText = false;
        return;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] SetText: oldLength=%zu, newLength=%zu", m_text.size(), text.size());

    m_text = text;
    m_caret.position = std::min(m_caret.position, m_text.size());
    m_textDirty = true;
    if (resetScrollOffset) {
        if (GetFinalWidth() > 0 && GetFinalHeight() > 0) {
            SetScrollOffset(0, 0);
        }
    }
    InvalidateVisual();
    TextChanged.Invoke(m_text);
    m_isUpdatingText = false;
}

void TextBox::OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                           const Properties::PropertyMetadata& metadata,
                                           const std::any& oldValue,
                                           const std::any& newValue,
                                           Properties::PropertyValueSource source)
{
    (void)metadata;
    (void)oldValue;
    (void)source;

    if (property.GetName() == "Text") {
        if (newValue.has_value()) {
            const std::wstring textValue = NeoWPF::UI::Properties::CoerceToWString(newValue);
            if (!m_isUpdatingText && m_text != textValue) {
                SetText(textValue);
                return;
            }
        }
    }

    Content::ContentControl::OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);
}

void TextBox::SetPlaceholderText(const std::wstring& placeholder)
{
    if (IsApplyingPropertyValue()) {
        m_placeholderText = placeholder;
        m_textDirty = true;
        InvalidateVisual();
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("PlaceholderText", *this);
    if (property) {
        SetValue(*property, placeholder);
        return;
    }

    if (m_placeholderText == placeholder) {
        return;
    }

    m_placeholderText = placeholder;
    m_textDirty = true;
    InvalidateVisual();
}

const std::wstring& TextBox::GetPlaceholderText() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("PlaceholderText", *this);
    if (property) {
        try {
            const std::wstring value = GetValue<std::wstring>(*property);
            if (m_placeholderText != value) {
                const_cast<TextBox*>(this)->m_placeholderText = value;
            }
            return m_placeholderText;
        } catch (const std::bad_any_cast&) {}
    }
    return m_placeholderText;
}

void TextBox::SetTextColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_textColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Foreground", *this);
    if (property) { SetValue(*property, color); return; }
    m_textColor = color; InvalidateVisual();
}

void TextBox::SetFontFamily(const std::wstring& fontFamily) noexcept
{   
    if (IsApplyingPropertyValue()) {
        m_fontFamily = fontFamily;
        m_textDirty = true;
        InvalidateVisual();
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontFamily", *this);
    if (property) {
        SetValue(*property, fontFamily);
        return;
    }

    if (m_fontFamily == fontFamily) {
        return;
    }

    m_fontFamily = fontFamily;
    m_textDirty = true;
    InvalidateVisual();
}

std::wstring TextBox::GetFontFamily() const noexcept
{
    // dependency props can override the local field, so we check them first
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontFamily", *this);
    if (property) {
        try {
            const auto value = GetValue<std::wstring>(*property);
            return value;
        } catch (const std::bad_any_cast&) {}
    }
    return m_fontFamily;
}

void TextBox::SetFontWeight(int fontWeight) noexcept
{
    if (fontWeight < 100) {
        fontWeight = 100;
    }
    if (fontWeight > 1000) {
        fontWeight = 1000;
    }

    if (m_fontWeight == fontWeight) {
        return;
    }

    m_fontWeight = fontWeight;
    m_textDirty = true;
    InvalidateMeasure();
    InvalidateVisual();
}

int TextBox::GetFontWeight() const noexcept
{
    return m_fontWeight;
}

void TextBox::SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept
{
    if (m_fontStyle == fontStyle) {
        return;
    }

    m_fontStyle = fontStyle;
    m_textDirty = true;
    InvalidateMeasure();
    InvalidateVisual();
}

NeoWPF::UI::FontStyle TextBox::GetFontStyle() const noexcept
{
    return m_fontStyle;
}

void TextBox::SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept
{
    if (m_textHorizontalAlignment == alignment) {
        return;
    }
    m_textHorizontalAlignment = alignment;
    InvalidateVisual();
}

TextHorizontalAlignment TextBox::GetTextHorizontalAlignment() const noexcept
{
    return m_textHorizontalAlignment;
}

void TextBox::SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept
{
    if (m_textVerticalAlignment == alignment) {
        return;
    }
    m_textVerticalAlignment = alignment;
    InvalidateVisual();
}

TextVerticalAlignment TextBox::GetTextVerticalAlignment() const noexcept
{
    return m_textVerticalAlignment;
}

void TextBox::SetTextColor(std::uint32_t color) noexcept
{
    SetTextColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetTextColor() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Foreground", *this);
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) { return m_textColor; }
    }
    return m_textColor;
}

void TextBox::SetBackgroundColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBackgroundColor(color);
    InvalidateVisual();
}

void TextBox::OnPaddingChanged(Thickness padding)
{
    if (auto presenter = GetPresenter()) {
        presenter->SetPadding(padding);
    }
    InvalidateVisual();
}

void TextBox::OnBackgroundColorChanged(NeoWPF::UI::Color color)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetFillColor(color.ToArgb());
    }
    InvalidateVisual();
}

void TextBox::OnBorderColorChanged(NeoWPF::UI::Color color)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetBorderColor(color.ToArgb());
    }
    InvalidateVisual();
}

void TextBox::OnBorderThicknessChanged(int thickness)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetBorderThickness(thickness);
    }
    InvalidateVisual();
}

void TextBox::SetBackgroundColor(std::uint32_t color) noexcept
{
    SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetBackgroundColor() const noexcept
{
    return Control::GetBackgroundColor();
}

void TextBox::SetFontSize(int fontSize) noexcept
{
    if (IsApplyingPropertyValue()) { m_fontSize = fontSize; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontSize", *this);
    if (property) { SetValue(*property, fontSize); return; }
    m_fontSize = fontSize; InvalidateVisual();
}

int TextBox::GetFontSize() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontSize", *this);
    if (property) {
        try { return GetValue<int>(*property); } catch (const std::bad_any_cast&) { return m_fontSize; }
    }
    return m_fontSize;
}

void TextBox::SetPadding(Thickness padding) noexcept
{
    Control::SetPadding(padding);
    InvalidateVisual();
}

Thickness TextBox::GetPadding() const noexcept
{
    return Control::GetPadding();
}

void TextBox::SetBorderColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBorderColor(color);
    InvalidateVisual();
}

void TextBox::SetBorderColor(std::uint32_t color) noexcept
{
    SetBorderColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetBorderColor() const noexcept
{
    return Control::GetBorderColor();
}

void TextBox::SetCaretColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_caretColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CaretBrush", *this);
    if (property) { SetValue(*property, color); return; }
    m_caretColor = color; InvalidateVisual();
}

void TextBox::SetCaretColor(std::uint32_t color) noexcept
{
    SetCaretColor(NeoWPF::UI::Color::FromArgb(color));
}

NeoWPF::UI::Color TextBox::GetCaretColor() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("CaretBrush", *this);
    if (property) {
        try { return GetValue<NeoWPF::UI::Color>(*property); } catch (const std::bad_any_cast&) { return m_caretColor; }
    }
    return m_caretColor;
}

void TextBox::Measure(int availableWidth, int availableHeight)
{
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, GetFontFamily(), 0, 0);
    const int minWidth = 120;
    const int minHeight = std::max(baseMetrics.height, m_fontSize) + (GetPadding().Horizontal() + GetPadding().Vertical());
    int desiredWidth = std::max(minWidth, availableWidth > 0 ? std::min(minWidth, availableWidth) : minWidth);
    int desiredHeight = std::max(minHeight, availableHeight > 0 ? std::min(minHeight, availableHeight) : minHeight);

    {
        const auto displayText = m_text.empty() && !m_placeholderText.empty() ? m_placeholderText : m_text;
        int widthConstraint = 0;
        if (m_textWrapping != TextWrapping::NoWrap && availableWidth > 0) {
            widthConstraint = std::max(0, availableWidth - GetPadding().Horizontal());
        }
        auto flow = NeoWPF::UI::Text::BreakText(displayText, m_fontSize, GetFontFamily(), widthConstraint, m_textWrapping, m_fontWeight, m_fontStyle);
        desiredHeight = std::max(minHeight, flow.contentHeight + GetPadding().Vertical());
        desiredWidth = std::max(desiredWidth, flow.contentWidth + GetPadding().Horizontal());
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] Measure: available=%dx%d, desired=%dx%d, textLength=%zu", availableWidth, availableHeight, desiredWidth, desiredHeight, m_text.size());

    if (auto presenter = GetPresenter()) {
        presenter->Measure(availableWidth, availableHeight);
    }

    const int resolvedWidth = ResolveWidth(availableWidth, desiredWidth);
    const int resolvedHeight = ResolveHeight(availableHeight, desiredHeight);
    SetDesiredSize(resolvedWidth, resolvedHeight);
}

void TextBox::Arrange(int finalWidth, int finalHeight)
{
    Content::ContentControl::Arrange(finalWidth, finalHeight);

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] Arrange: final=%dx%d, textDirty=%d, lineCount=%zu", finalWidth, finalHeight, m_textDirty ? 1 : 0, m_renderCache.GetLines().size());
    if (m_textDirty) {
        EnsureTextTexture();
    }
}

void TextBox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (GetFinalWidth() <= 0 || GetFinalHeight() <= 0) {
        return;
    }

    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    auto border = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Border"));
    if (border) {
        border->SetRect(absX, absY, GetFinalWidth(), GetFinalHeight());
        border->SetBorderThickness(1);
        border->SetCornerRadius(GetCornerRadius());
        border->SetOpacity(GetOpacity());
        border->BuildRenderCommands(commands);
    }
    else {
        const auto cornerRadius = GetCornerRadius();
        if (GetBorderColor().ToArgb() != 0x00000000u) {
            commands.DrawRoundedRect(
                static_cast<float>(absX),
                static_cast<float>(absY),
                static_cast<float>(GetFinalWidth()),
                static_cast<float>(GetFinalHeight()),
                cornerRadius.TopLeft,
                cornerRadius.TopRight,
                cornerRadius.BottomRight,
                cornerRadius.BottomLeft,
                m_borderColor.ToArgb(),
                GetOpacity());
        }

        if (GetBackgroundColor().ToArgb() != 0x00000000u) {
            commands.DrawRoundedRect(
                static_cast<float>(absX + 1),
                static_cast<float>(absY + 1),
                static_cast<float>(std::max(0, GetFinalWidth() - 2)),
                static_cast<float>(std::max(0, GetFinalHeight() - 2)),
                std::max(0.0f, cornerRadius.TopLeft - 1.0f),
                std::max(0.0f, cornerRadius.TopRight - 1.0f),
                std::max(0.0f, cornerRadius.BottomRight - 1.0f),
                std::max(0.0f, cornerRadius.BottomLeft - 1.0f),
                m_backgroundColor.ToArgb(),
                GetOpacity());
        }
    }

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    const int contentHostX = absX + GetPadding().Left;
    const int contentHostY = absY + GetPadding().Top;
    const int contentHostWidth = std::max(0, GetFinalWidth() - GetPadding().Horizontal());
    const int contentHostHeight = std::max(0, GetFinalHeight() - GetPadding().Vertical());

    if (contentHostWidth <= 0 || contentHostHeight <= 0) {
        return;
    }

    if (contentHost) {
        const int inset = std::max(0, GetPadding().Left);
        contentHost->SetRect(absX + inset, absY + inset, contentHostWidth, contentHostHeight);
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHost->BuildRenderCommands(commands);
    }

    const auto clip = NeoWPF::Rendering::ClipRect{ static_cast<float>(contentHostX), static_cast<float>(contentHostY), static_cast<float>(contentHostWidth), static_cast<float>(contentHostHeight), true };

    const bool showingPlaceholder = m_text.empty() && !m_placeholderText.empty();
    auto drawColor = GetTextColor();
    if (showingPlaceholder) {
        drawColor.A = static_cast<std::uint8_t>(std::max(0u, static_cast<unsigned>(GetTextColor().A) / 2u));
        if (drawColor.A < 0x40u) {
            drawColor.A = 0x40u;
        }
    }

    const auto& lines = m_renderCache.GetLines();
    if (lines.empty()) {
        return;
    }

    const int verticalAlignmentOffset = TextAlignment::GetBlockVerticalOffset(
        m_renderCache.GetContentWidth(), m_renderCache.GetContentHeight(), contentHostHeight, m_textVerticalAlignment);
    int currentY = verticalAlignmentOffset - m_scrollOffsetY;

    for (std::size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
        const auto& line = lines[lineIndex];
        const auto textureId = m_renderCache.GetLineTextureId(lineIndex);
        const int lineHeight = std::max(1, line.lineHeight);
        if (textureId == 0 || line.width <= 0 || line.height <= 0) {
            currentY += lineHeight;
            continue;
        }

        const int lineOffsetX = TextAlignment::GetLineHorizontalOffset(
            line.width, contentHostWidth, m_textHorizontalAlignment);
        const int lineDrawX = contentHostX + lineOffsetX - m_scrollOffsetX;
        const int lineDrawY = contentHostY + currentY - line.inkTop;

        commands.DrawTexturedQuad(
            static_cast<float>(lineDrawX),
            static_cast<float>(lineDrawY),
            static_cast<float>(line.width),
            static_cast<float>(line.height),
            0.0f, 0.0f,
            1.0f, 1.0f,
            textureId,
            drawColor.ToArgb(),
            GetOpacity(),
            clip);

        currentY += lineHeight;
    }

    if (m_isFocused) {
        int caretX = GetCaretX();
        int caretY = GetCaretY();
        if (caretY < 0 || caretX < 0) {
            caretX = std::max(0, caretX);
            caretY = std::max(0, caretY);
        }
        const int caretHeight = std::max(1, GetCaretHeight());

        const uint32_t caretTextureId = GetCaretTextureId();
        commands.DrawTexturedQuad(
            static_cast<float>(contentHostX + caretX - m_scrollOffsetX),
            static_cast<float>(contentHostY + verticalAlignmentOffset + caretY - m_scrollOffsetY),
            1.0f,
            static_cast<float>(caretHeight),
            0.0f, 0.0f,
            1.0f, 1.0f,
            caretTextureId,
            GetTextColor().ToArgb(),
            GetOpacity(),
            clip);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

void TextBox::EnsureTextTexture()
{
    if (!m_textDirty) {
        return;
    }

    const int contentHostWidth = std::max(0, GetFinalWidth() - GetPadding().Horizontal());
    if (contentHostWidth <= 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] EnsureTextTexture: content host width not ready; skipping texture refresh");
        return;
    }

    int widthConstraint = 0;
    if (m_textWrapping != TextWrapping::NoWrap) {
        widthConstraint = contentHostWidth;
    }

    const auto displayText = (m_text.empty() && !m_placeholderText.empty()) ? m_placeholderText : m_text;
    m_renderCache.Refresh(displayText, m_fontSize, GetFontFamily(), widthConstraint, m_textWrapping);
    m_textDirty = false;
    InvalidateVisual();
}



}
