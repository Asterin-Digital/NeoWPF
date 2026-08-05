#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace NeoWPF::UI {

 

Border::Border()
{
    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    SetPresenter(presenter);

    ApplyBorderThemeStyle(this);
}

void Border::Measure(int availableWidth, int availableHeight)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] available=%dx%d explicit=%dx%d padding=%d border=%d",
        availableWidth, availableHeight, GetWidth(), GetHeight(), GetPadding(), GetBorderThickness());

    if (auto presenter = GetPresenter()) {
        if (!presenter->GetParent()) {
            try {
                AddChild(presenter);
            } catch (const std::bad_weak_ptr&) {
            }
        }

        const int explicitW = GetWidth();
        const int explicitH = GetHeight();
        const int availableContentWidth = std::max(0, explicitW > 0 ? explicitW : availableWidth);
        const int availableContentHeight = std::max(0, explicitH > 0 ? explicitH : availableHeight);

        const int innerWidth = std::max(0, availableContentWidth - (2 * GetBorderThickness()) - (2 * GetPadding()));
        const int innerHeight = std::max(0, availableContentHeight - (2 * GetBorderThickness()) - (2 * GetPadding()));

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] inner=%dx%d content=%dx%d",
            innerWidth, innerHeight, availableContentWidth, availableContentHeight);

        presenter->Measure(innerWidth, innerHeight);

        const int contentW = presenter->GetDesiredWidth();
        const int contentH = presenter->GetDesiredHeight();
        const int contentDesiredW = std::max(0, contentW + (2 * GetBorderThickness()) + (2 * GetPadding()));
        const int contentDesiredH = std::max(0, contentH + (2 * GetBorderThickness()) + (2 * GetPadding()));

        const int desiredW = std::max(contentDesiredW, explicitW > 0 ? explicitW : 0);
        const int desiredH = std::max(contentDesiredH, explicitH > 0 ? explicitH : 0);

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] desired=%dx%d content=%dx%d",
            desiredW, desiredH, contentDesiredW, contentDesiredH);

        SetDesiredSize(desiredW, desiredH);
        return;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] No presenter, desired=0x0");

    SetDesiredSize(0, 0);
}

void Border::Arrange(int finalWidth, int finalHeight)
{
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Arrange] final=%dx%d padding=%d border=%d",
        finalWidth, finalHeight, GetPadding(), GetBorderThickness());

    FrameworkElement::Arrange(finalWidth, finalHeight);

    if (auto presenter = GetPresenter()) {
        const int innerX = GetBorderThickness() + GetPadding();
        const int innerY = GetBorderThickness() + GetPadding();
        const int innerWidth = std::max(0, finalWidth - (2 * GetBorderThickness()) - (2 * GetPadding()));
        const int innerHeight = std::max(0, finalHeight - (2 * GetBorderThickness()) - (2 * GetPadding()));

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Arrange] presenter offset=%dx%d inner=%dx%d",
            innerX, innerY, innerWidth, innerHeight);

        presenter->Arrange(innerWidth, innerHeight);
        presenter->SetLayoutOffset(innerX, innerY);
        /* if (!presenter->GetParent()) {
            try {
                AddChild(presenter);
            } catch (const std::bad_weak_ptr&) {
            }
        } */ // possible crash
    }
}

void Border::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!IsVisible()) {
        return;
    }

    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    const auto cornerRadius = GetCornerRadius();

    if (GetBorderThickness() > 0 && GetBorderColor().ToArgb() != 0x00000000u) {
        const float borderInset = static_cast<float>(GetBorderThickness()) * 0.5f;
        commands.DrawRoundedRect(
            static_cast<float>(absX + borderInset),
            static_cast<float>(absY + borderInset),
            static_cast<float>(std::max(0, GetFinalWidth() - (2 * static_cast<int>(std::lround(borderInset))))),
            static_cast<float>(std::max(0, GetFinalHeight() - (2 * static_cast<int>(std::lround(borderInset))))),
            std::max(0.0f, cornerRadius.TopLeft - borderInset),
            std::max(0.0f, cornerRadius.TopRight - borderInset),
            std::max(0.0f, cornerRadius.BottomRight - borderInset),
            std::max(0.0f, cornerRadius.BottomLeft - borderInset),
            GetBorderColor().ToArgb(),
            GetOpacity());
    }

    if (GetBackgroundColor().ToArgb() != 0x00000000u) {
        const float inset = static_cast<float>(GetBorderThickness());
        commands.DrawRoundedRect(
            static_cast<float>(absX + inset),
            static_cast<float>(absY + inset),
            static_cast<float>(std::max(0, GetFinalWidth() - (2 * GetBorderThickness()))),
            static_cast<float>(std::max(0, GetFinalHeight() - (2 * GetBorderThickness()))),
            std::max(0.0f, cornerRadius.TopLeft - inset),
            std::max(0.0f, cornerRadius.TopRight - inset),
            std::max(0.0f, cornerRadius.BottomRight - inset),
            std::max(0.0f, cornerRadius.BottomLeft - inset),
            GetBackgroundColor().ToArgb(),
            GetOpacity());
    }

    ContentControl::BuildRenderCommands(commands);
}

void Border::SetPadding(int padding) noexcept
{
    Control::SetPadding(padding);
    InvalidateVisual();
}

int Border::GetPadding() const noexcept
{
    return Control::GetPadding();
}

void Border::SetBorderThickness(int thickness) noexcept
{
    Control::SetBorderThickness(thickness);
    InvalidateVisual();
}

int Border::GetBorderThickness() const noexcept
{
    return Control::GetBorderThickness();
}

void Border::SetBorderColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBorderColor(color);
    InvalidateVisual();
}

NeoWPF::UI::Color Border::GetBorderColor() const noexcept
{
    return Control::GetBorderColor();
}

void Border::SetBackgroundColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBackgroundColor(color);
    InvalidateVisual();
}

NeoWPF::UI::Color Border::GetBackgroundColor() const noexcept
{
    return Control::GetBackgroundColor();
}

void Border::OnPaddingChanged(int)
{
    InvalidateVisual();
}

void Border::OnBorderThicknessChanged(int)
{
    InvalidateVisual();
}

void Border::OnBorderColorChanged(NeoWPF::UI::Color)
{
    InvalidateVisual();
}

void Border::OnBackgroundColorChanged(NeoWPF::UI::Color)
{
    InvalidateVisual();
}

}
