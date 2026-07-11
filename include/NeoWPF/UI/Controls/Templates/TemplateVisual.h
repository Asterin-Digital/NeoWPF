#pragma once

#include <cstdint>
#include <string>

#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Media/CornerRadius.h>

namespace NeoWPF::UI::Templates {

class TemplateVisual : public NeoWPF::UI::Visual {
public:
    explicit TemplateVisual(const std::string& name = {});

    void SetRect(int x, int y, int width, int height) noexcept;
    int GetX() const noexcept { return m_x; }
    int GetY() const noexcept { return m_y; }
    int GetWidth() const noexcept { return m_width; }
    int GetHeight() const noexcept { return m_height; }
    void SetFillColor(std::uint32_t color) noexcept;
    void SetBorderColor(std::uint32_t color) noexcept;
    void SetBorderThickness(int thickness) noexcept;
    void SetCornerRadius(NeoWPF::UI::CornerRadius cornerRadius) noexcept;
    NeoWPF::UI::CornerRadius GetCornerRadius() const noexcept;
    void SetOpacity(float opacity) noexcept;
    float GetOpacity() const noexcept;
    void SetClipRect(const NeoWPF::Rendering::ClipRect& clip) noexcept;

    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

private:
    std::string m_name;
    int m_x = 0;
    int m_y = 0;
    int m_width = 0;
    int m_height = 0;
    std::uint32_t m_fillColor = 0x00000000u;
    std::uint32_t m_borderColor = 0x00000000u;
    int m_borderThickness = 0;
    NeoWPF::UI::CornerRadius m_cornerRadius;
    float m_opacity = 1.0f;
    NeoWPF::Rendering::ClipRect m_clip{};
};

} // namespace NeoWPF::UI::Templates
