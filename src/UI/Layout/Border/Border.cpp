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
    SetClipToBounds(true);
    ApplyBorderThemeStyle(this);
}

void Border::Measure(int availableWidth, int availableHeight)
{
    const auto padding = GetPadding();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] available=%dx%d explicit=%dx%d padding=(%d,%d,%d,%d) border=%d",
        availableWidth, availableHeight, GetWidth(), GetHeight(),
        padding.Left, padding.Top, padding.Right, padding.Bottom, GetBorderThickness());

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

        const auto border = GetBorderThicknessValue();
        const int innerWidth = std::max(0, availableContentWidth - border.Left - border.Right - padding.Horizontal());
        const int innerHeight = std::max(0, availableContentHeight - border.Top - border.Bottom - padding.Vertical());

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] inner=%dx%d content=%dx%d",
            innerWidth, innerHeight, availableContentWidth, availableContentHeight);

        presenter->Measure(innerWidth, innerHeight);

        const int contentW = presenter->GetDesiredWidth();
        const int contentH = presenter->GetDesiredHeight();
        const int contentDesiredW = std::max(0, contentW + border.Left + border.Right + padding.Horizontal());
        const int contentDesiredH = std::max(0, contentH + border.Top + border.Bottom + padding.Vertical());

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
    const auto padding = GetPadding();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Arrange] final=%dx%d padding=(%d,%d,%d,%d) border=%d",
        finalWidth, finalHeight, padding.Left, padding.Top, padding.Right, padding.Bottom, GetBorderThickness());

    FrameworkElement::Arrange(finalWidth, finalHeight);

    if (auto presenter = GetPresenter()) {
        const auto border = GetBorderThicknessValue();
        const int innerX = border.Left + padding.Left;
        const int innerY = border.Top + padding.Top;
        const int innerWidth = std::max(0, finalWidth - border.Left - border.Right - padding.Horizontal());
        const int innerHeight = std::max(0, finalHeight - border.Top - border.Bottom - padding.Vertical());

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

    const auto clip = GetClipRect();

    if (GetBorderThickness() > 0 && GetBorderColor().ToArgb() != 0x00000000u) {
        const auto border = GetBorderThicknessValue();
        if (border.Top > 0) commands.DrawRect(static_cast<float>(absX), static_cast<float>(absY), static_cast<float>(GetFinalWidth()), static_cast<float>(border.Top), GetBorderColor().ToArgb(), GetOpacity(), clip);
        if (border.Bottom > 0) commands.DrawRect(static_cast<float>(absX), static_cast<float>(absY + GetFinalHeight() - border.Bottom), static_cast<float>(GetFinalWidth()), static_cast<float>(border.Bottom), GetBorderColor().ToArgb(), GetOpacity(), clip);
        if (border.Left > 0) commands.DrawRect(static_cast<float>(absX), static_cast<float>(absY + border.Top), static_cast<float>(border.Left), static_cast<float>(std::max(0, GetFinalHeight() - border.Top - border.Bottom)), GetBorderColor().ToArgb(), GetOpacity(), clip);
        if (border.Right > 0) commands.DrawRect(static_cast<float>(absX + GetFinalWidth() - border.Right), static_cast<float>(absY + border.Top), static_cast<float>(border.Right), static_cast<float>(std::max(0, GetFinalHeight() - border.Top - border.Bottom)), GetBorderColor().ToArgb(), GetOpacity(), clip);
    }

    if (GetBackgroundColor().ToArgb() != 0x00000000u) {
        const auto border = GetBorderThicknessValue();
        const float inset = static_cast<float>(std::max({border.Left, border.Top, border.Right, border.Bottom}));
        commands.DrawRoundedRect(
            static_cast<float>(absX + border.Left),
            static_cast<float>(absY + border.Top),
            static_cast<float>(std::max(0, GetFinalWidth() - border.Left - border.Right)),
            static_cast<float>(std::max(0, GetFinalHeight() - border.Top - border.Bottom)),
            std::max(0.0f, cornerRadius.TopLeft - inset),
            std::max(0.0f, cornerRadius.TopRight - inset),
            std::max(0.0f, cornerRadius.BottomRight - inset),
            std::max(0.0f, cornerRadius.BottomLeft - inset),
            GetBackgroundColor().ToArgb(),
            GetOpacity(),
            clip,
            0.0f);
    }

    ContentControl::BuildRenderCommands(commands);
}

void Border::SetPadding(Thickness padding) noexcept
{
    Control::SetPadding(padding);
    InvalidateVisual();
}

Thickness Border::GetPadding() const noexcept
{
    return Control::GetPadding();
}

void Border::SetBorderThickness(int thickness) noexcept
{
    Control::SetBorderThickness(thickness);
    m_borderThicknesses = Thickness(thickness);
    InvalidateVisual();
}

int Border::GetBorderThickness() const noexcept
{
    return Control::GetBorderThickness();
}

void Border::SetBorderThickness(Thickness thickness) noexcept
{
    m_borderThicknesses = thickness;
    Control::SetBorderThickness(std::max({thickness.Left, thickness.Top, thickness.Right, thickness.Bottom}));
    InvalidateVisual();
}

Thickness Border::GetBorderThicknessValue() const noexcept
{
    return m_borderThicknesses;
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

void Border::OnPaddingChanged(Thickness)
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
