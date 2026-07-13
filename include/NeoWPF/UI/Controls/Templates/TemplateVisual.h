#pragma once

#include <cstdint>
#include <string>

#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/UI/Core/Element/Visual.h>
#include <NeoWPF/UI/Core/Media/CornerRadius.h>

namespace NeoWPF::UI::Templates {

class TemplateVisual : public NeoWPF::UI::Visual {
public:
    /*
        @summary
        Creates a new template visual.

        @param name
        The visual name.
    */
    explicit TemplateVisual(const std::string& name = {});

    /*
        @summary
        Sets the visual rectangle.

        @param x
        The x position.

        @param y
        The y position.

        @param width
        The visual width.

        @param height
        The visual height.
    */
    void SetRect(int x, int y, int width, int height) noexcept;

    /*
        @summary
        Returns the visual x position.

        @returns
        The x position in pixels.
    */
    int GetX() const noexcept { return m_x; }

    /*
        @summary
        Returns the visual y position.

        @returns
        The y position in pixels.
    */
    int GetY() const noexcept { return m_y; }

    /*
        @summary
        Returns the visual width.

        @returns
        The width in pixels.
    */
    int GetWidth() const noexcept { return m_width; }

    /*
        @summary
        Returns the visual height.

        @returns
        The height in pixels.
    */
    int GetHeight() const noexcept { return m_height; }

    /*
        @summary
        Sets the fill color.

        @param color
        The fill color as an ARGB value.
    */
    void SetFillColor(std::uint32_t color) noexcept;

    /*
        @summary
        Sets the border color.

        @param color
        The border color as an ARGB value.
    */
    void SetBorderColor(std::uint32_t color) noexcept;

    /*
        @summary
        Sets the border thickness.

        @param thickness
        The border thickness in pixels.
    */
    void SetBorderThickness(int thickness) noexcept;

    /*
        @summary
        Sets the corner radius.

        @param cornerRadius
        The corner radius values.
    */
    void SetCornerRadius(NeoWPF::UI::CornerRadius cornerRadius) noexcept;

    /*
        @summary
        Returns the corner radius.

        @returns
        The current corner radius.
    */
    NeoWPF::UI::CornerRadius GetCornerRadius() const noexcept;

    /*
        @summary
        Sets the opacity.

        @param opacity
        The opacity value.
    */
    void SetOpacity(float opacity) noexcept;

    /*
        @summary
        Returns the opacity.

        @returns
        The current opacity.
    */
    float GetOpacity() const noexcept;

    /*
        @summary
        Sets the clip rectangle.

        @param clip
        The clip rectangle to apply.
    */
    void SetClipRect(const NeoWPF::Rendering::ClipRect& clip) noexcept;

    /*
        @summary
        Sets the rotation.

        @param rotationRadians
        The rotation in radians.
    */
    void SetRotation(float rotationRadians) noexcept;

    /*
        @summary
        Returns the rotation.

        @returns
        The current rotation in radians.
    */
    float GetRotation() const noexcept;

    /*
        @summary
        Builds the render commands for the visual.

        @param commands
        The command list to fill.

        @internal
    */
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
    float m_rotation = 0.0f;
};

} // namespace NeoWPF::UI::Templates
