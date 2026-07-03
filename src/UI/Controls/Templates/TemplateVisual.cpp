#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>

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
    MarkTreeDirty();
}

void TemplateVisual::SetFillColor(std::uint32_t color) noexcept
{
    m_fillColor = color;
    MarkTreeDirty();
}

void TemplateVisual::SetBorderColor(std::uint32_t color) noexcept
{
    m_borderColor = color;
    MarkTreeDirty();
}

void TemplateVisual::SetBorderThickness(int thickness) noexcept
{
    m_borderThickness = thickness < 0 ? 0 : thickness;
    MarkTreeDirty();
}

void TemplateVisual::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (m_width <= 0 || m_height <= 0) {
        return;
    }

    commands.DrawRect(
        static_cast<float>(m_x),
        static_cast<float>(m_y),
        static_cast<float>(m_width),
        static_cast<float>(m_height),
        m_fillColor);

    if (m_borderThickness <= 0) {
        return;
    }

    const int thickness = m_borderThickness < 1 ? 1 : m_borderThickness;
    commands.DrawRect(
        static_cast<float>(m_x),
        static_cast<float>(m_y),
        static_cast<float>(m_width),
        static_cast<float>(thickness),
        m_borderColor);
    commands.DrawRect(
        static_cast<float>(m_x),
        static_cast<float>(m_y + m_height - thickness),
        static_cast<float>(m_width),
        static_cast<float>(thickness),
        m_borderColor);
    commands.DrawRect(
        static_cast<float>(m_x),
        static_cast<float>(m_y),
        static_cast<float>(thickness),
        static_cast<float>(m_height),
        m_borderColor);
    commands.DrawRect(
        static_cast<float>(m_x + m_width - thickness),
        static_cast<float>(m_y),
        static_cast<float>(thickness),
        static_cast<float>(m_height),
        m_borderColor);
}

} // namespace NeoWPF::UI::Templates
