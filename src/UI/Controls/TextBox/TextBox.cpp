#include <NeoWPF/UI/Controls/TextBox/TextBox.h>
#include <NeoWPF/UI/Controls/TextBoxStyles.h>
#include <NeoWPF/ControlStyles.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
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

    APPLY_CONTROL_STYLES(NeoWPF::UI::Generated::TextBoxStyles, *this);

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(GetPadding());
    SetPresenter(presenter);


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
    NeoWPF::UI::Localization::LocaleOperations::ClearLocale(*this, TextProperty());
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

void TextBox::SetLocale(const std::wstring& key)
{
    NeoWPF::UI::Localization::LocaleOperations::SetLocale(*this, TextProperty(), key);
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

void TextBox::OnBorderThicknessChanged(double thickness)
{
    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetBorderThickness(std::lround(thickness));
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

void TextBox::Measure(double availableWidth, double availableHeight)
{
    const auto baseMetrics = NeoWPF::Rendering::Text::MeasureText(L"W", m_fontSize, GetFontFamily(), 0, 0);
    const double minWidth = 120.0;
    const double minHeight = baseMetrics.height > m_fontSize ? baseMetrics.height : m_fontSize;
    const double adjustedMinHeight = minHeight + (GetPadding().Horizontal() + GetPadding().Vertical());
    double desiredWidth = minWidth;
    if (availableWidth > 0.0 && availableWidth < minWidth) {
        desiredWidth = availableWidth;
    }
    double desiredHeight = adjustedMinHeight;
    if (availableHeight > 0.0 && availableHeight < adjustedMinHeight) {
        desiredHeight = availableHeight;
    }

    {
        const auto displayText = m_text.empty() && !m_placeholderText.empty() ? m_placeholderText : m_text;
        double widthConstraint = 0.0;
        if (m_textWrapping != TextWrapping::NoWrap && availableWidth > 0.0) {
            widthConstraint = std::max(0.0, availableWidth - GetPadding().Horizontal());
        }
        auto flow = NeoWPF::UI::Text::BreakText(displayText, m_fontSize, GetFontFamily(), widthConstraint, m_textWrapping, m_fontWeight, m_fontStyle);
        desiredHeight = std::max(minHeight, flow.contentHeight + GetPadding().Vertical());
        desiredWidth = std::max(desiredWidth, flow.contentWidth + GetPadding().Horizontal());
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Info, "[TextBox] Measure: available=%dx%d, desired=%dx%d, textLength=%zu", availableWidth, availableHeight, desiredWidth, desiredHeight, m_text.size());

    if (auto presenter = GetPresenter()) {
        presenter->Measure(availableWidth, availableHeight);
    }

    const double resolvedWidth = ResolveWidth(availableWidth, desiredWidth);
    const double resolvedHeight = ResolveHeight(availableHeight, desiredHeight);
    SetDesiredSize(resolvedWidth, resolvedHeight);
}

void TextBox::Arrange(double finalWidth, double finalHeight)
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

    const auto renderTransform = GetWorldRenderTransform();
    double absX = 0.0;
    double absY = 0.0;
    renderTransform.positionMatrix.TransformPoint(0.0, 0.0, absX, absY);

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
                absX,
                absY,
                GetFinalWidth(),
                GetFinalHeight(),
                cornerRadius.TopLeft,
                cornerRadius.TopRight,
                cornerRadius.BottomRight,
                cornerRadius.BottomLeft,
                m_borderColor.ToArgb(),
                GetOpacity(),
                NeoWPF::Rendering::ClipRect{},
                renderTransform.rotationRadians,
                renderTransform.pivot.X,
                renderTransform.pivot.Y);
            }
        if (GetBackgroundColor().ToArgb() != 0x00000000u) {
            const double innerWidth = GetFinalWidth() - 2.0 > 0.0 ? GetFinalWidth() - 2.0 : 0.0;
            const double innerHeight = GetFinalHeight() - 2.0 > 0.0 ? GetFinalHeight() - 2.0 : 0.0;
            const float radiusTL = cornerRadius.TopLeft > 1.0f ? cornerRadius.TopLeft - 1.0f : 0.0f;
            const float radiusTR = cornerRadius.TopRight > 1.0f ? cornerRadius.TopRight - 1.0f : 0.0f;
            const float radiusBR = cornerRadius.BottomRight > 1.0f ? cornerRadius.BottomRight - 1.0f : 0.0f;
            const float radiusBL = cornerRadius.BottomLeft > 1.0f ? cornerRadius.BottomLeft - 1.0f : 0.0f;
            const double innerRenderWidth = innerWidth;
            const double innerRenderHeight = innerHeight;
            commands.DrawRoundedRect(
                absX + 1.0f,
                absY + 1.0f,
                innerRenderWidth,
                innerRenderHeight,
                radiusTL,
                radiusTR,
                radiusBR,
                radiusBL,
                m_backgroundColor.ToArgb(),
                GetOpacity(),
                NeoWPF::Rendering::ClipRect{},
                renderTransform.rotationRadians,
                renderTransform.pivot.X,
                renderTransform.pivot.Y);
        }
    }

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    const double contentHostX = absX + GetPadding().Left;
    const double contentHostY = absY + GetPadding().Top;
    const double contentHostWidth = GetFinalWidth() - GetPadding().Horizontal() > 0.0 ? GetFinalWidth() - GetPadding().Horizontal() : 0.0;
    const double contentHostHeight = GetFinalHeight() - GetPadding().Vertical() > 0.0 ? GetFinalHeight() - GetPadding().Vertical() : 0.0;

    if (contentHostWidth <= 0 || contentHostHeight <= 0) {
        return;
    }

    if (contentHost) {
        const double inset = GetPadding().Left > 0.0 ? GetPadding().Left : 0.0;
        contentHost->SetRect(absX + inset, absY + inset, contentHostWidth, contentHostHeight);
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHost->BuildRenderCommands(commands);
    }

    const double clipX = contentHostX;
    const double clipY = contentHostY;
    const double clipW = contentHostWidth;
    const double clipH = contentHostHeight;
    const auto clip = NeoWPF::Rendering::ClipRect{ clipX, clipY, clipW, clipH, true };

    const bool showingPlaceholder = m_text.empty() && !m_placeholderText.empty();
    auto drawColor = GetTextColor();
    if (showingPlaceholder) {
        const auto textAlpha = GetTextColor().A / 2u;
        drawColor.A = textAlpha > 0u ? textAlpha : 0u;
        if (drawColor.A < 0x40u) {
            drawColor.A = 0x40u;
        }
    }

    const auto& lines = m_renderCache.GetLines();
    if (lines.empty()) {
        return;
    }

    const double verticalAlignmentOffset = TextAlignment::GetBlockVerticalOffset(
        m_renderCache.GetContentWidth(), m_renderCache.GetContentHeight(), contentHostHeight, m_textVerticalAlignment);
    double currentY = verticalAlignmentOffset - m_scrollOffsetY;

    for (std::size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
        const auto& line = lines[lineIndex];
        const auto textureId = m_renderCache.GetLineTextureId(lineIndex);
        const double lineHeight = line.lineHeight > 1.0 ? line.lineHeight : 1.0;
        if (textureId == 0 || line.width <= 0 || line.height <= 0) {
            currentY += lineHeight;
            continue;
        }

        const double lineOffsetX = TextAlignment::GetLineHorizontalOffset(
            line.width, contentHostWidth, m_textHorizontalAlignment);
        const double lineDrawX = contentHostX + lineOffsetX - m_scrollOffsetX;
        const double lineDrawY = contentHostY + currentY - line.inkTop;

        commands.DrawTexturedQuad(
            lineDrawX,
            lineDrawY,
            line.width,
            line.height,
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
            caretX = caretX > 0 ? caretX : 0;
            caretY = caretY > 0 ? caretY : 0;
        }
        const int caretHeight = GetCaretHeight() > 1 ? GetCaretHeight() : 1;

        const uint32_t caretTextureId = GetCaretTextureId();
        commands.DrawTexturedQuad(
            contentHostX + caretX - m_scrollOffsetX,
            contentHostY + verticalAlignmentOffset + caretY - m_scrollOffsetY,
            1.0f,
            caretHeight,
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

    const double hostWidth = GetFinalWidth() - GetPadding().Horizontal();
    const auto contentHostWidth = std::lround(hostWidth > 0.0 ? hostWidth : 0.0);
    if (contentHostWidth <= 0) {
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::TextBox, NeoWPF::Diagnostics::Level::Debug, "[TextBox] EnsureTextTexture: content host width not ready; skipping texture refresh");
        return;
    }

    const auto displayText = (m_text.empty() && !m_placeholderText.empty()) ? m_placeholderText : m_text;
    (void)displayText;
    m_renderCache.Clear();
    m_textDirty = false;
    InvalidateVisual();
}



}
