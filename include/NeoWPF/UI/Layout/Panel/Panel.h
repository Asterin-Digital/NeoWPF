/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/UI/Core/Media/Color.h>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::Input::Core
{
    class MouseEventArgs;
}

namespace NeoWPF::UI {

class Panel : public Control {
public:
    Panel();

    static const NeoWPF::UI::Properties::DependencyProperty& BackgroundColorProperty();

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept;
    void SetBackgroundColor(std::uint32_t color) noexcept { SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color)); }
    NeoWPF::UI::Color GetBackgroundColor() const noexcept;

protected:
    void OnBackgroundColorChanged(NeoWPF::UI::Color color) override;
};

}
