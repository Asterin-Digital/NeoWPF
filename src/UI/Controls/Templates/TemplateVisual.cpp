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

void TemplateVisual::SetRect(int x, int y, int width, int height) noexcept
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

void TemplateVisual::SetRotation(float rotationRadians) noexcept
{
    m_rotation = rotationRadians;
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

    const float x = static_cast<float>(m_x);
    const float y = static_cast<float>(m_y);
    const float width = static_cast<float>(m_width);
    const float height = static_cast<float>(m_height);

    if (m_borderThickness > 0 && m_borderColor != 0x00000000u) {
        const float borderInset = static_cast<float>(std::max(0, m_borderThickness)) * 0.5f;
        const float borderX = x + borderInset;
        const float borderY = y + borderInset;
        const float borderWidth = std::max(0.0f, width - (2.0f * borderInset));
        const float borderHeight = std::max(0.0f, height - (2.0f * borderInset));

        commands.DrawRoundedRect(
            borderX,
            borderY,
            borderWidth,
            borderHeight,
            std::max(0.0f, m_cornerRadius.TopLeft - borderInset),
            std::max(0.0f, m_cornerRadius.TopRight - borderInset),
            std::max(0.0f, m_cornerRadius.BottomRight - borderInset),
            std::max(0.0f, m_cornerRadius.BottomLeft - borderInset),
            m_borderColor,
            m_opacity,
            m_clip,
            m_rotation);
    }

    if (m_fillColor != 0x00000000u) {
        const float inset = static_cast<float>(std::max(0, m_borderThickness));
        const float innerX = x + inset;
        const float innerY = y + inset;
        const float innerWidth = std::max(0.0f, width - (2.0f * inset));
        const float innerHeight = std::max(0.0f, height - (2.0f * inset));
        if (innerWidth > 0.0f && innerHeight > 0.0f) {
            commands.DrawRoundedRect(
                innerX,
                innerY,
                innerWidth,
                innerHeight,
                std::max(0.0f, m_cornerRadius.TopLeft - inset),
                std::max(0.0f, m_cornerRadius.TopRight - inset),
                std::max(0.0f, m_cornerRadius.BottomRight - inset),
                std::max(0.0f, m_cornerRadius.BottomLeft - inset),
                m_fillColor,
                m_opacity,
                m_clip,
                m_rotation);
        }
    }
}

} // namespace NeoWPF::UI::Templates
