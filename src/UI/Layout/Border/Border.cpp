#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>

namespace NeoWPF::UI {

namespace {

void ApplyBorderTheme(Border& border)
{
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;
    using NeoWPF::Core::ApplicationThemes::ThemeBorderThickness;
    using NeoWPF::Core::ApplicationThemes::ThemeCornerRadius;
    using NeoWPF::Core::ApplicationThemes::ThemePadding;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;

    auto& themeManager = ThemeManager::Instance();
    border.SetThemePropertyValue("Background",
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Surface));
    border.SetThemePropertyValue("BorderBrush",
        NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Border));

    NeoWPF::UI::Thickness padding;
    if (themeManager.TryGetPadding(ThemePadding::Border, padding)) {
        border.SetThemePropertyValue("Padding", padding);
    }

    NeoWPF::UI::Thickness borderThickness;
    if (themeManager.TryGetBorderThickness(ThemeBorderThickness::Border, borderThickness)) {
        border.SetThemePropertyValue("BorderThickness", borderThickness);
    }

    NeoWPF::UI::CornerRadius cornerRadius;
    if (themeManager.TryGetCornerRadius(ThemeCornerRadius::Control, cornerRadius)) {
        border.SetThemePropertyValue("CornerRadius", cornerRadius);
    }
}

}

Border::Border()
{
    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    SetPresenter(presenter);
    SetClipToBounds(true);
    ApplyBorderTheme(*this);
    m_themeAppliedSubscriptionId = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
        ApplyBorderTheme(*this);
    });
}

Border::~Border()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(m_themeAppliedSubscriptionId);
}

void Border::Measure(double availableWidth, double availableHeight)
{
    const auto padding = GetPadding();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] available=%fx%f explicit=%fx%f padding=(%f,%f,%f,%f) border=%f",
        availableWidth, availableHeight, GetWidth(), GetHeight(),
        padding.Left, padding.Top, padding.Right, padding.Bottom, GetBorderThickness());

    if (auto presenter = GetPresenter()) {
        if (!presenter->GetParent()) {
            try {
                AddChild(presenter);
            } catch (const std::bad_weak_ptr&) {
            }
        }

        const double explicitW = GetWidth();
        const double explicitH = GetHeight();
        const double availableContentWidth = (std::max)(0.0, explicitW > 0.0 ? explicitW : availableWidth);
        const double availableContentHeight = (std::max)(0.0, explicitH > 0.0 ? explicitH : availableHeight);

        const auto border = GetBorderThicknessValue();
        const double innerWidth = (std::max)(0.0, availableContentWidth - border.Left - border.Right - padding.Horizontal());
        const double innerHeight = (std::max)(0.0, availableContentHeight - border.Top - border.Bottom - padding.Vertical());

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] inner=%fx%f content=%fx%f",
            innerWidth, innerHeight, availableContentWidth, availableContentHeight);

        presenter->Measure(innerWidth, innerHeight);

        const double contentW = presenter->GetDesiredWidth();
        const double contentH = presenter->GetDesiredHeight();
        const double contentDesiredW = (std::max)(0.0, contentW + border.Left + border.Right + padding.Horizontal());
        const double contentDesiredH = (std::max)(0.0, contentH + border.Top + border.Bottom + padding.Vertical());

        const double desiredW = (std::max)(contentDesiredW, explicitW > 0.0 ? explicitW : 0.0);
        const double desiredH = (std::max)(contentDesiredH, explicitH > 0.0 ? explicitH : 0.0);

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] desired=%fx%f content=%fx%f",
            desiredW, desiredH, contentDesiredW, contentDesiredH);

        SetDesiredSize(desiredW, desiredH);
        return;
    }

    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Measure] No presenter, desired=0x0");

    SetDesiredSize(0, 0);
}

void Border::Arrange(double finalWidth, double finalHeight)
{
    const auto padding = GetPadding();
    NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Arrange] final=%fx%f padding=(%f,%f,%f,%f) border=%f",
        finalWidth, finalHeight, padding.Left, padding.Top, padding.Right, padding.Bottom, GetBorderThickness());

    FrameworkElement::Arrange(finalWidth, finalHeight);

    if (auto presenter = GetPresenter()) {
        const auto border = GetBorderThicknessValue();
        const double innerX = border.Left + padding.Left;
        const double innerY = border.Top + padding.Top;
        const double innerWidth = (std::max)(0.0, finalWidth - border.Left - border.Right - padding.Horizontal());
        const double innerHeight = (std::max)(0.0, finalHeight - border.Top - border.Bottom - padding.Vertical());

        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Visual, NeoWPF::Diagnostics::Level::Info, "[Border::Arrange] presenter offset=%fx%f inner=%fx%f",
            innerX, innerY, innerWidth, innerHeight);

        presenter->Arrange(innerWidth, innerHeight);
        presenter->SetLayoutOffset(innerX, innerY);
        /* if (!presenter->GetParent()) {
            try {
                AddChild(presenter);
            } catch (const std::bad_weak_ptr&) {
            }
        } */ // bad dont do this
    }
}

void Border::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    if (!IsVisible()) {
        return;
    }

    double absX = 0.0;
    double absY = 0.0;
    GetAbsolutePosition(absX, absY);

    const auto cornerRadius = GetCornerRadius();

    const auto clip = GetClipRect();

    const auto borderBrush = std::dynamic_pointer_cast<SolidColorBrush>(GetBorderBrush());
    const auto backgroundBrush = std::dynamic_pointer_cast<SolidColorBrush>(GetBackgroundBrush());
    if (GetBorderThickness() > 0 && borderBrush && borderBrush->GetArgb() != 0x00000000u) {
        const auto border = GetBorderThicknessValue();
        const auto opacity = static_cast<float>(GetOpacity() * borderBrush->GetOpacity());
        if (border.Top > 0) commands.DrawRect(absX, absY, GetFinalWidth(), border.Top, borderBrush->GetArgb(), opacity, clip);
        if (border.Bottom > 0) commands.DrawRect(absX, absY + GetFinalHeight() - border.Bottom, GetFinalWidth(), border.Bottom, borderBrush->GetArgb(), opacity, clip);
        if (border.Left > 0) commands.DrawRect(absX, absY + border.Top, border.Left, (std::max)(0.0, GetFinalHeight() - border.Top - border.Bottom), borderBrush->GetArgb(), opacity, clip);
        if (border.Right > 0) commands.DrawRect(absX + GetFinalWidth() - border.Right, absY + border.Top, border.Right, (std::max)(0.0, GetFinalHeight() - border.Top - border.Bottom), borderBrush->GetArgb(), opacity, clip);
    }

    if (backgroundBrush && backgroundBrush->GetArgb() != 0x00000000u) {
        const auto border = GetBorderThicknessValue();
        const double inset = std::max({border.Left, border.Top, border.Right, border.Bottom});
        commands.DrawRoundedRect(
            absX + border.Left,
            absY + border.Top,
            (std::max)(0.0, GetFinalWidth() - border.Left - border.Right),
            (std::max)(0.0, GetFinalHeight() - border.Top - border.Bottom),
            std::max(0.0, cornerRadius.TopLeft - inset),
            std::max(0.0, cornerRadius.TopRight - inset),
            std::max(0.0, cornerRadius.BottomRight - inset),
            std::max(0.0, cornerRadius.BottomLeft - inset),
            backgroundBrush->GetArgb(),
            static_cast<float>(GetOpacity() * backgroundBrush->GetOpacity()),
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

void Border::SetBorderThickness(double thickness) noexcept
{
    Control::SetBorderThickness(thickness);
    m_borderThicknesses = Thickness(thickness);
    InvalidateVisual();
}

double Border::GetBorderThickness() const noexcept
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

void Border::OnPaddingChanged(Thickness)
{
    InvalidateVisual();
}

void Border::OnBorderThicknessChanged(double)
{
    InvalidateVisual();
}

}
