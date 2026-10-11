#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Core/Media/SolidColorBrush.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>
#include <memory>

namespace NeoWPF::UI {

namespace {

void ApplyCheckboxThemeStyle(
    Checkbox& checkbox)
{
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;
    using NeoWPF::Core::ApplicationThemes::ThemeCornerRadius;
    using NeoWPF::Core::ApplicationThemes::ThemeFontSize;
    using NeoWPF::Core::ApplicationThemes::ThemePadding;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;
    auto& themeManager = ThemeManager::Instance();
    checkbox.SetThemePropertyValue("Foreground", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Foreground));
    checkbox.SetThemePropertyValue("Background", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Surface));
    checkbox.SetThemePropertyValue("BorderBrush", NeoWPF::Core::ApplicationThemes::GetPaletteBrush(PaletteBrush::Border));

    int fontSize = 0;
    if (themeManager.TryGetFontSize(ThemeFontSize::Control, fontSize)) {
        checkbox.SetThemePropertyValue("FontSize", fontSize);
    }

    NeoWPF::UI::Thickness padding;
    if (themeManager.TryGetPadding(ThemePadding::Checkbox, padding)) {
        checkbox.SetThemePropertyValue("Padding", padding);
    }

    NeoWPF::UI::CornerRadius cornerRadius;
    if (themeManager.TryGetCornerRadius(ThemeCornerRadius::Checkbox, cornerRadius)) {
        checkbox.SetThemePropertyValue("CornerRadius", cornerRadius);
    }
}

}

Checkbox::Checkbox()
    : ToggleBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<CheckboxTemplate>());

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    SetPresenter(presenter);

    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    m_textContent->SetVerticalAlignment(VerticalAlignment::Stretch);
    m_textContent->SetText(L"");
    m_textContent->SetTextHorizontalAlignment(TextHorizontalAlignment::Left);
    m_textContent->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    m_textContent->SetForeground(GetForeground());
    m_textContent->SetFontSize(m_fontSize);
    SetContent(m_textContent);
    ApplyCheckboxThemeStyle(*this);
    m_themeAppliedSubscriptionId = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
        ApplyCheckboxThemeStyle(*this);
            ApplyVisualState(m_currentVisualState, false);
    });
}

Checkbox::~Checkbox()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(m_themeAppliedSubscriptionId);
    NeoWPF::UI::Localization::LocaleOperations::ClearLocale(*this, TextProperty());
}

void Checkbox::SetChecked(bool value) noexcept
{
    if (IsApplyingPropertyValue()) {
        ToggleBase::SetChecked(value);
        return;
    }

    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("IsChecked", *this);
    if (property) {
        SetValue(*property, value);
        return;
    }

    ToggleBase::SetChecked(value);
}

void Checkbox::SetText(const std::wstring& text)
{
    if (IsApplyingPropertyValue()) { m_text = text; if (m_textContent) { m_textContent->SetText(text); } InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Text", *this);
    if (property) { SetValue(*property, text); return; }
    m_text = text;
    if (m_textContent) {
        m_textContent->SetText(text);
    }
    InvalidateVisual();
}

void Checkbox::SetLocale(const std::wstring& key)
{
    NeoWPF::UI::Localization::LocaleOperations::SetLocale(*this, TextProperty(), key);
}

void Checkbox::SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept
{
    if (m_textContent) {
        m_textContent->SetTextHorizontalAlignment(alignment);
    }
    InvalidateVisual();
}

TextHorizontalAlignment Checkbox::GetTextHorizontalAlignment() const noexcept
{
    if (m_textContent) {
        return m_textContent->GetTextHorizontalAlignment();
    }
    return TextHorizontalAlignment::Left;
}

void Checkbox::SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept
{
    if (m_textContent) {
        m_textContent->SetTextVerticalAlignment(alignment);
    }
    InvalidateVisual();
}

TextVerticalAlignment Checkbox::GetTextVerticalAlignment() const noexcept
{
    if (m_textContent) {
        return m_textContent->GetTextVerticalAlignment();
    }
    return TextVerticalAlignment::Top;
}

void Checkbox::SetForeground(std::shared_ptr<Brush> brush)
{
    if (!IsApplyingPropertyValue()) {
        SetValue(ForegroundProperty(), brush);
        return;
    }
    if (m_textContent) {
        m_textContent->SetForeground(brush);
    }
    InvalidateVisual();
}

void Checkbox::SetPadding(Thickness padding) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_padding = padding;
        InvalidateVisual();
        return;
    }
    SetValue(PaddingProperty(), padding);
}

void Checkbox::SetFontSize(int fontSize) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_fontSize = fontSize;
        if (m_textContent) {
            m_textContent->SetFontSize(fontSize);
        }
        InvalidateVisual();
        return;
    }
    SetValue(FontSizeProperty(), fontSize);
}

void Checkbox::SetFontWeight(int fontWeight) noexcept
{
    fontWeight = (std::clamp)(fontWeight, 100, 1000);
    if (IsApplyingPropertyValue()) {
        m_fontWeight = fontWeight;
        if (m_textContent) {
            m_textContent->SetFontWeight(fontWeight);
        }
        InvalidateVisual();
        return;
    }
    SetValue(FontWeightProperty(), fontWeight);
}

int Checkbox::GetFontWeight() const noexcept
{
    return GetValue<int>(FontWeightProperty());
}

void Checkbox::SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept
{
    if (IsApplyingPropertyValue()) {
        m_fontStyle = fontStyle;
        if (m_textContent) {
            m_textContent->SetFontStyle(fontStyle);
        }
        InvalidateVisual();
        return;
    }
    SetValue(FontStyleProperty(), fontStyle);
}

NeoWPF::UI::FontStyle Checkbox::GetFontStyle() const noexcept
{
    return GetValue<NeoWPF::UI::FontStyle>(FontStyleProperty());
}

std::shared_ptr<Brush> Checkbox::GetForeground() const
{
    return GetValue<std::shared_ptr<Brush>>(ForegroundProperty());
}

Thickness Checkbox::GetPadding() const noexcept
{
    return GetValue<Thickness>(PaddingProperty());
}

int Checkbox::GetFontSize() const noexcept
{
    return GetValue<int>(FontSizeProperty());
}

void Checkbox::Measure(double availableWidth, double availableHeight)
{
    const auto padding = GetPadding();
    const double contentAvailableWidth = availableWidth > 0.0 ? (std::max)(0.0, availableWidth - padding.Horizontal()) : 0.0;
    Content::ContentControl::Measure(contentAvailableWidth, 0);

    double contentWidth = 0.0;
    double contentHeight = 0.0;
    if (auto presenter = GetPresenter()) {
        contentWidth = presenter->GetDesiredWidth();
        contentHeight = presenter->GetDesiredHeight();
    }

    const double boxSize = (std::max)(18.0, contentHeight);
    const double desiredWidth = boxSize + padding.Horizontal() + contentWidth;
    const double desiredHeight = (std::max)(boxSize, contentHeight) + padding.Vertical();

    SetDesiredSize(desiredWidth, desiredHeight);
}

void Checkbox::Arrange(double finalWidth, double finalHeight)
{
    const auto padding = GetPadding();
    const double contentHeight = (std::max)(0.0, finalHeight);
    Content::ContentControl::Arrange(finalWidth, finalHeight);

    if (auto presenter = GetPresenter()) {
        const double desiredOffsetX = 18.0 + padding.Left;
        if (presenter->GetLayoutOffsetX() != desiredOffsetX || presenter->GetLayoutOffsetY() != padding.Top) {
            presenter->SetLayoutOffset(desiredOffsetX, padding.Top);
        }
        presenter->Arrange((std::max)(0.0, finalWidth - (18.0 + padding.Left + padding.Right)),
            (std::max)(0.0, contentHeight - padding.Top - padding.Bottom));
        if (auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(presenter->FindTemplateChild("PART_ContentHost"))) {
            const double desiredTextOffsetY = (finalHeight - contentHeight) / 2.0;
            contentHost->SetRect(
                presenter->GetLayoutOffsetX(),
                desiredTextOffsetY + padding.Top,
                (std::max)(0.0, finalWidth - presenter->GetLayoutOffsetX()),
                contentHeight);
        }
    }
}

void Checkbox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const double boxSize = 18.0;

    const auto renderTransform = GetWorldRenderTransform();
    double fAbsX = 0.0;
    double fAbsY = 0.0;
    double drawWidth = GetFinalWidth();
    double drawHeight = GetFinalHeight();
    renderTransform.positionMatrix.TransformPoint(fAbsX, fAbsY, fAbsX, fAbsY);
    double right = drawWidth;
    double bottom = drawHeight;
    renderTransform.positionMatrix.TransformPoint(right, bottom, right, bottom);
    drawWidth = std::abs(right - fAbsX);
    drawHeight = std::abs(bottom - fAbsY);

    const double boxTop = fAbsY + (drawHeight - boxSize) / 2.0;
    const double boxLeft = fAbsX;

    auto box = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Box"));
    if (box) {
        box->SetRect(boxLeft, boxTop, boxSize, boxSize);
        box->SetCornerRadius(GetCornerRadius());
        box->SetBorderThickness(1);
        box->SetOpacity(GetOpacity());
        box->BuildRenderCommands(commands);
    }
    else {
        const auto radius = GetCornerRadius();
        const auto background = std::dynamic_pointer_cast<SolidColorBrush>(GetBackgroundBrush());
        const auto border = std::dynamic_pointer_cast<SolidColorBrush>(GetBorderBrush());
        commands.DrawRoundedRect(boxLeft, boxTop, boxSize, boxSize, radius.TopLeft, radius.TopRight, radius.BottomRight, radius.BottomLeft, background ? background->GetArgb() : 0u, GetOpacity(), NeoWPF::Rendering::ClipRect{}, renderTransform.rotationRadians, renderTransform.pivot.X, renderTransform.pivot.Y);
        commands.DrawRoundedRect(boxLeft, boxTop, boxSize, boxSize, radius.TopLeft, radius.TopRight, radius.BottomRight, radius.BottomLeft, border ? border->GetArgb() : 0u, GetOpacity(), NeoWPF::Rendering::ClipRect{}, renderTransform.rotationRadians, renderTransform.pivot.X, renderTransform.pivot.Y);
    }

    if (IsChecked()) {
        const auto foreground = std::dynamic_pointer_cast<SolidColorBrush>(GetForeground());
        const double checkCenterX = boxLeft + boxSize * 0.5;
        const double checkCenterY = boxTop + boxSize * 0.5;
        constexpr float kCheckAngle = 0.68f;

        auto drawCheckStroke = [&](double width, double height, float rotationRadians, double offsetX, double offsetY) {
            const double x = checkCenterX + offsetX - width * 0.5;
            const double y = checkCenterY + offsetY - height * 0.5;
            commands.DrawRoundedRect(x, y, width, height, 0.0f, 0.0f, 0.0f, 0.0f,
                                    foreground ? foreground->GetArgb() : 0u, GetOpacity(),
                                    NeoWPF::Rendering::ClipRect{}, rotationRadians,
                                    renderTransform.pivot.X, renderTransform.pivot.Y);
        };

        drawCheckStroke(2.0f, 5.0f, -kCheckAngle, -2.5f, 1.0f);
        drawCheckStroke(8.0f, 1.5f, -kCheckAngle, 1.8f, -1.0f);
    }

    const auto padding = GetPadding();
    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const double hostX = fAbsX + boxSize + padding.Left;
        const double hostY = fAbsY + padding.Top;
        const double hostWidth = (std::max)(0.0, drawWidth - boxSize - padding.Left - padding.Right);
        const double hostHeight = (std::max)(1.0, drawHeight - padding.Top - padding.Bottom);
        contentHost->SetRect(hostX, hostY, hostWidth, hostHeight);
        contentHost->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

void Checkbox::OnVisualStateApplied(const std::string& stateName)
{
    (void)stateName;
}

}
