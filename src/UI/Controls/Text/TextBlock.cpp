#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>

namespace NeoWPF::UI {

namespace {

float GetAbsoluteRenderTransformRotation(const UIElement* element)
{
    using namespace NeoWPF::UI::Core;
    Matrix4 matrix = Matrix4::Identity();
    const UIElement* current = element;
    while (current) {
        if (auto fe = dynamic_cast<const FrameworkElement*>(current)) {
            matrix = matrix * Matrix4::Translation(
                static_cast<float>(fe->GetLayoutOffsetX() + fe->GetRenderOffsetX()),
                static_cast<float>(fe->GetLayoutOffsetY() + fe->GetRenderOffsetY()));
        }

        if (auto transform = current->GetRenderTransform()) {
            float width = 0.0f;
            float height = 0.0f;
            if (auto fe = dynamic_cast<const FrameworkElement*>(current)) {
                width = static_cast<float>((std::max)(fe->GetFinalWidth(), fe->GetDesiredWidth()));
                height = static_cast<float>((std::max)(fe->GetFinalHeight(), fe->GetDesiredHeight()));
            }

            const auto origin = current->GetRenderTransformOrigin();
            const float originX = width * origin.X;
            const float originY = height * origin.Y;

            if (originX != 0.0f || originY != 0.0f) {
                matrix = matrix * Matrix4::Translation(originX, originY);
                matrix = matrix * transform->GetMatrix();
                matrix = matrix * Matrix4::Translation(-originX, -originY);
            } else {
                matrix = matrix * transform->GetMatrix();
            }
        }

        current = dynamic_cast<const UIElement*>(current->GetParent());
    }

    return std::atan2(matrix.m[1][0], matrix.m[0][0]);
}

}

TextBlock::TextBlock()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
}

TextBlock::TextBlock(const std::wstring& text) : TextBlock()
{
    SetText(text);
}

TextBlock::~TextBlock()
{
    if (m_textureId != 0) {
        NeoWPF::Rendering::Renderer_DestroyTexture(m_textureId);
        m_textureId = 0;
    }
}

void TextBlock::SetText(const std::wstring& text)
{
    if (m_text == text) {
        return;
    }
    m_text = text;
    m_textDirty = true;
    InvalidateVisual();
}

void TextBlock::SetFontFamily(const std::wstring& fontFamily) noexcept
{
    if (fontFamily.empty()) {
        return;
    }

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

std::wstring TextBlock::GetFontFamily() const noexcept
{
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontFamily", *this);
    if (property) {
        try { return GetValue<std::wstring>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_fontFamily;
}

int TextBlock::GetPadding() const noexcept {
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", *this);
    if (property) {
        try { return GetValue<int>(*property); } catch (const std::bad_any_cast&) {}
    }
    return m_padding;
}

void TextBlock::SetPadding(int padding) noexcept {
    if (IsApplyingPropertyValue()) {
        m_padding = padding;
        OnPaddingChanged(padding);
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", *this);
    if (property) {
        if (GetPadding() != padding) {
            SetValue(*property, padding);
        }
        return;
    }

    m_padding = padding;
    OnPaddingChanged(padding);
}

void TextBlock::OnPaddingChanged(int padding) {
    InvalidateMeasure();
    InvalidateVisual();
}

void TextBlock::Measure(int availableWidth, int availableHeight)
{
    const int padding = GetPadding();
    if (m_text.empty()) {
        SetDesiredSize(padding * 2, padding * 2);
        return;
    }

    const int widthConstraint = availableWidth > 0 ? std::max(0, availableWidth - (2 * padding)) : 0;
    const int heightConstraint = availableHeight > 0 ? std::max(0, availableHeight - (2 * padding)) : 0;
    auto metrics = NeoWPF::Rendering::Text::MeasureText(m_text, m_fontSize, GetFontFamily(), widthConstraint, heightConstraint);
    SetDesiredSize(metrics.width + (2 * padding), metrics.height + (2 * padding));
}

void TextBlock::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);
    const int innerWidth = std::max(0, finalWidth - (2 * GetPadding()));
    const int innerHeight = std::max(0, finalHeight - (2 * GetPadding()));
    if (m_textDirty || innerWidth != m_lastArrangeWidth || innerHeight != m_lastArrangeHeight) {
        EnsureTextTexture(innerWidth, innerHeight);
        m_lastArrangeWidth = innerWidth;
        m_lastArrangeHeight = innerHeight;
    }
}

void TextBlock::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_text.empty() || m_textWidth == 0 || m_textHeight == 0 || m_textureId == 0) {
        return;
    }

    const auto clip = GetClipRect();
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    float drawX = static_cast<float>(absX);
    float drawY = static_cast<float>(absY);
    float drawWidth = static_cast<float>(m_textWidth);
    float drawHeight = static_cast<float>(m_textHeight);
    const int padding = GetPadding();
    drawX += static_cast<float>(padding);
    drawY += static_cast<float>(padding);
    ApplyRenderTransform(drawX, drawY, drawWidth, drawHeight);
    float rotationRadians = GetAbsoluteRenderTransformRotation(this);

    commands.DrawTexturedQuad(
        drawX,
        drawY,
        drawWidth,
        drawHeight,
        0.0f, 0.0f,
        1.0f, 1.0f,
        m_textureId,
        m_textColor.ToArgb(),
        GetOpacity(),
        clip,
        rotationRadians);
}

void TextBlock::EnsureTextTexture(int availableWidth, int availableHeight)
{
    if (!m_textDirty && availableWidth == m_lastArrangeWidth && availableHeight == m_lastArrangeHeight) {
        return;
    }

    if (m_textureId != 0) {
        NeoWPF::Rendering::Renderer_DestroyTexture(m_textureId);
        m_textureId = 0;
    }

    if (m_text.empty()) {
        m_textWidth = 0;
        m_textHeight = 0;
        m_textDirty = false;
        return;
    }

    int width = 0;
    int height = 0;
    const int INF = 1000000;
    const int widthConstraint = INF;
    const int heightConstraint = availableHeight > 0 ? availableHeight : INF;
    auto bitmap = NeoWPF::Rendering::Text::RenderTextBitmap(m_text, m_fontSize, widthConstraint, heightConstraint, GetFontFamily(), width, height);
    if (bitmap.empty() || width == 0 || height == 0) {
        NeoWPF::Diagnostics::LogError("TextBlock: failed to render text bitmap");
        m_textWidth = 0;
        m_textHeight = 0;
        m_textDirty = false;
        return;
    }

    m_textureId = NeoWPF::Rendering::Renderer_CreateTexture(static_cast<unsigned int>(width), static_cast<unsigned int>(height), bitmap.data());
    m_textWidth = width;
    m_textHeight = height;
    m_textDirty = false;
}

}
