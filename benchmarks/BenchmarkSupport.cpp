#include "BenchmarkSupport.h"

#include <NeoWPF/Core/Application/Resources/Embedded/FallbackFont.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Layout/Panel/StackPanel.h>

#include <string_view>

namespace NeoWPF::Benchmarks {
namespace {

constexpr double kElementExtent = 8.0;

class BenchmarkElement final : public NeoWPF::UI::FrameworkElement {
public:
    void Measure(double, double) override
    {
        SetDesiredSize(kElementExtent, kElementExtent);
    }

    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override
    {
        commands.DrawRect(
            GetLayoutOffsetX(),
            GetLayoutOffsetY(),
            GetFinalWidth(),
            GetFinalHeight(),
            0xFF336699u);
    }
};

}

bool RegisterBenchmarkFont()
{
    using NeoWPF::Core::ApplicationResources::Embedded::FontRecord;
    using NeoWPF::Core::ApplicationResources::Embedded::GetEmbeddedFallbackFontCount;
    using NeoWPF::Core::ApplicationResources::Embedded::TryGetEmbeddedFallbackFont;
    using NeoWPF::Rendering::Text::RegisterFontMemory;

    for (std::size_t index = 0; index < GetEmbeddedFallbackFontCount(); ++index) {
        FontRecord font;
        if (TryGetEmbeddedFallbackFont(index, font) && font.data && font.size > 0 &&
            RegisterFontMemory(font.data, font.size, kBenchmarkFontFamily)) {
            return true;
        }
    }
    return false;
}

std::shared_ptr<NeoWPF::UI::StackPanel> CreateStackWorkload()
{
    auto panel = std::make_shared<NeoWPF::UI::StackPanel>();
    for (std::size_t index = 0; index < kStackChildCount; ++index) {
        panel->AddChild(std::make_shared<BenchmarkElement>());
    }

    panel->Measure(kLayoutWidth, kLayoutHeight);
    panel->Arrange(kLayoutWidth, kLayoutHeight);
    return panel;
}

std::wstring CreateTextWorkload(std::size_t repetitionCount)
{
    constexpr std::wstring_view phrase = L"Retained UI text layout benchmark. ";
    std::wstring text;
    text.reserve(repetitionCount * phrase.size());
    for (std::size_t index = 0; index < repetitionCount; ++index) {
        text.append(phrase);
    }
    return text;
}

}