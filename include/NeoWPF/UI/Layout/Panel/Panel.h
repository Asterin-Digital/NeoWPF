#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::Input::Core
{
    class MouseEventArgs;
}

namespace NeoWPF::UI {

static bool s_panelLogging;

void RegisterPanelStyles();

class Panel : public FrameworkElement {
public:
    void Measure(int availableWidth, int availableHeight) override;
    void Arrange(int finalWidth, int finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    virtual void OnMouseDown(int x, int y, std::uint32_t buttonState) override {}
    virtual void OnMouseUp(int x, int y, std::uint32_t buttonState) override {}

    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept { m_backgroundColor = color; InvalidateVisual(); }
    void SetBackgroundColor(std::uint32_t color) noexcept { SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color)); }
    NeoWPF::UI::Color GetBackgroundColor() const noexcept { return m_backgroundColor; }

private:
    NeoWPF::UI::Color m_backgroundColor = NeoWPF::UI::Color::FromArgb(0x00000000u);
};

}
