#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <algorithm>

namespace NeoWPF::UI::Templates {

TemplateVisual::TemplateVisual(const std::string& name)
    : m_name(name)
{
}

void TemplateVisual::SetRect(double x, double y, double width, double height) noexcept
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
}

void TemplateVisual::SetFillBrush(std::shared_ptr<NeoWPF::UI::Brush> brush) noexcept
{
    m_fillBrush = std::move(brush);
}

void TemplateVisual::SetBorderBrush(std::shared_ptr<NeoWPF::UI::Brush> brush) noexcept
{
    m_borderBrush = std::move(brush);
}

void TemplateVisual::SetBorderThickness(int thickness) noexcept
{
    m_borderThickness = thickness < 0 ? 0 : thickness;
}

void TemplateVisual::SetCornerRadius(NeoWPF::UI::CornerRadius cornerRadius) noexcept
{
    m_cornerRadius = cornerRadius;
}

NeoWPF::UI::CornerRadius TemplateVisual::GetCornerRadius() const noexcept
{
    return m_cornerRadius;
}

void TemplateVisual::SetOpacity(float opacity) noexcept
{
    if (opacity < 0.0f) {
        opacity = 0.0f;
    } else if (opacity > 1.0f) {
        opacity = 1.0f;
    }
    m_opacity = opacity;
}

float TemplateVisual::GetOpacity() const noexcept
{
    return m_opacity;
}

void TemplateVisual::SetClipRect(const NeoWPF::Rendering::ClipRect& clip) noexcept
{
    m_clip = clip;
}

void TemplateVisual::SetRotation(float rotationRadians, float pivotX, float pivotY) noexcept
{
    m_rotation = rotationRadians;
    m_rotationPivotX = pivotX;
    m_rotationPivotY = pivotY;
}

float TemplateVisual::GetRotation() const noexcept
{
    return m_rotation;
}

void TemplateVisual::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_width <= 0 || m_height <= 0) {
        return;
    }

    const double x = m_x;
    const double y = m_y;
    const double width = m_width;
    const double height = m_height;
    const auto borderPaint = NeoWPF::UI::ResolveBrushPaint(
        m_borderBrush, commands, x, y, width, height);
    const auto borderOpacity = m_opacity;
    const auto fillOpacity = m_opacity;

    if (m_borderThickness > 0 && !borderPaint.IsEmpty()) {
        commands.DrawRoundedRect(
            x,
            y,
            width,
            height,
            m_cornerRadius.TopLeft,
            m_cornerRadius.TopRight,
            m_cornerRadius.BottomRight,
            m_cornerRadius.BottomLeft,
            borderPaint,
            borderOpacity,
            m_clip,
            m_rotation,
            m_rotationPivotX,
            m_rotationPivotY);
    }

    const double inset = std::max<double>(0.0, m_borderThickness);
    const double innerX = x + inset;
    const double innerY = y + inset;
    const double innerWidth = std::max(0.0, width - (2.0 * inset));
    const double innerHeight = std::max(0.0, height - (2.0 * inset));
    const auto fillPaint = NeoWPF::UI::ResolveBrushPaint(
        m_fillBrush, commands, innerX, innerY, innerWidth, innerHeight);
    if (!fillPaint.IsEmpty()) {
        if (innerWidth > 0.0 && innerHeight > 0.0) {
            commands.DrawRoundedRect(
                innerX,
                innerY,
                innerWidth,
                innerHeight,
                std::max(0.0, m_cornerRadius.TopLeft - inset),
                std::max(0.0, m_cornerRadius.TopRight - inset),
                std::max(0.0, m_cornerRadius.BottomRight - inset),
                std::max(0.0, m_cornerRadius.BottomLeft - inset),
                fillPaint,
                fillOpacity,
                m_clip,
                m_rotation,
                m_rotationPivotX,
                m_rotationPivotY);
        }
    }
}

}
