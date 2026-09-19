#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <algorithm>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

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

void TemplateVisual::SetFillColor(std::uint32_t color) noexcept
{
    m_fillColor = color;
}

void TemplateVisual::SetBorderColor(std::uint32_t color) noexcept
{
    m_borderColor = color;
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

    if (m_borderThickness > 0 && m_borderColor != 0x00000000u) {
        commands.DrawRoundedRect(
            x,
            y,
            width,
            height,
            m_cornerRadius.TopLeft,
            m_cornerRadius.TopRight,
            m_cornerRadius.BottomRight,
            m_cornerRadius.BottomLeft,
            m_borderColor,
            m_opacity,
            m_clip,
            m_rotation,
            m_rotationPivotX,
            m_rotationPivotY);
    }

    if (m_fillColor != 0x00000000u) {
        const double inset = std::max<double>(0.0, m_borderThickness);
        const double innerX = x + inset;
        const double innerY = y + inset;
        const double innerWidth = std::max(0.0, width - (2.0 * inset));
        const double innerHeight = std::max(0.0, height - (2.0 * inset));
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
                m_fillColor,
                m_opacity,
                m_clip,
                m_rotation,
                m_rotationPivotX,
                m_rotationPivotY);
        }
    }
}

}
