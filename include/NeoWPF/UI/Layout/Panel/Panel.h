/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/Control.h>
#include <NeoWPF/Rendering/Renderer.h>

namespace NeoWPF::UI {

class Panel : public Control {
public:
    Panel();

    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

};

}
