#include <NeoWPF/UI/Controls/ToggleSwitch/ToggleSwitch.h>
#include <NeoWPF/Core/Application/ApplicationTheme.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <algorithm>
#include <memory>

namespace NeoWPF::UI {

namespace {

struct ToggleSwitchMetrics {
    double TrackWidth = 36.0;
    double TrackHeight = 20.0;
    double ThumbInset = 2.0;
    double LabelSpacing = 8.0;
};

ToggleSwitchMetrics GetToggleSwitchMetrics()
{
    ToggleSwitchMetrics metrics;
    auto& themeManager = NeoWPF::Core::ApplicationThemes::ThemeManager::Instance();
    themeManager.TryGetMetric(NeoWPF::Core::ApplicationThemes::ThemeMetric::ToggleSwitchTrackWidth,
                              metrics.TrackWidth);
    themeManager.TryGetMetric(NeoWPF::Core::ApplicationThemes::ThemeMetric::ToggleSwitchTrackHeight,
                              metrics.TrackHeight);
    themeManager.TryGetMetric(NeoWPF::Core::ApplicationThemes::ThemeMetric::ToggleSwitchThumbInset,
                              metrics.ThumbInset);
    themeManager.TryGetMetric(NeoWPF::Core::ApplicationThemes::ThemeMetric::ToggleSwitchLabelSpacing,
                              metrics.LabelSpacing);
    metrics.TrackWidth = (std::max)(0.0, metrics.TrackWidth);
    metrics.TrackHeight = (std::max)(0.0, metrics.TrackHeight);
    metrics.ThumbInset = (std::max)(0.0, metrics.ThumbInset);
    metrics.LabelSpacing = (std::max)(0.0, metrics.LabelSpacing);
    return metrics;
}

}

ToggleSwitch::ToggleSwitch()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<ToggleSwitchTemplate>());

    auto presenter = std::make_shared<Content::ContentPresenter>();
    SetPresenter(presenter);
    m_contentTextElement = std::make_shared<TextBlock>();
    m_contentTextElement->SetText(GetContentText());
    m_contentTextElement->SetTextHorizontalAlignment(TextHorizontalAlignment::Left);
    m_contentTextElement->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    SetContent(m_contentTextElement);

    ApplyTheme();
    m_themeAppliedSubscriptionId =
        NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().SubscribeThemeApplied([this]() {
            ApplyTheme();
        });
}

ToggleSwitch::~ToggleSwitch()
{
    NeoWPF::Core::ApplicationThemes::ThemeManager::Instance().UnsubscribeThemeApplied(
        m_themeAppliedSubscriptionId);
}

void ToggleSwitch::SetChecked(bool value) noexcept
{
    if (IsApplyingPropertyValue()) {
        ToggleBase::SetChecked(value);
        return;
    }

    SetValue(IsCheckedProperty(), value);
}

void ToggleSwitch::SetContentText(const std::wstring& text)
{
    if (!IsApplyingPropertyValue()) {
        SetValue(ContentTextProperty(), text);
        return;
    }

    ToggleBase::SetContentText(text);
    if (m_contentTextElement) {
        m_contentTextElement->SetText(text);
    }
    InvalidateMeasure();
    InvalidateVisual();
}

void ToggleSwitch::Measure(double availableWidth, double availableHeight)
{
    const auto metrics = GetToggleSwitchMetrics();
    const double labelAvailableWidth = availableWidth > 0.0
        ? (std::max)(0.0, availableWidth - metrics.TrackWidth - metrics.LabelSpacing)
        : 0.0;
    Content::ContentControl::Measure(labelAvailableWidth, availableHeight);

    double labelWidth = 0.0;
    double labelHeight = 0.0;
    if (!GetContentText().empty()) {
        if (const auto presenter = GetPresenter()) {
            labelWidth = presenter->GetDesiredWidth();
            labelHeight = presenter->GetDesiredHeight();
        }
    }

    const double labelOffset = GetContentText().empty() ? 0.0 : metrics.LabelSpacing;
    SetDesiredSize(metrics.TrackWidth + labelOffset + labelWidth,
                   (std::max)(metrics.TrackHeight, labelHeight));
}

void ToggleSwitch::Arrange(double finalWidth, double finalHeight)
{
    Content::ContentControl::Arrange(finalWidth, finalHeight);

    if (const auto presenter = GetPresenter()) {
        const auto metrics = GetToggleSwitchMetrics();
        const double labelOffset = metrics.TrackWidth +
            (GetContentText().empty() ? 0.0 : metrics.LabelSpacing);
        if (presenter->GetLayoutOffsetX() != labelOffset || presenter->GetLayoutOffsetY() != 0.0) {
            presenter->SetLayoutOffset(labelOffset, 0.0);
        }
        presenter->Arrange((std::max)(0.0, finalWidth - labelOffset), finalHeight);
    }
}

void ToggleSwitch::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const auto renderTransform = GetWorldRenderTransform();
    double absoluteX = 0.0;
    double absoluteY = 0.0;
    renderTransform.positionMatrix.TransformPoint(0.0f, 0.0f, absoluteX, absoluteY);

    auto track = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Track"));
    auto thumb = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Thumb"));
    const double width = (std::max)(0.0, GetFinalWidth());
    const double height = (std::max)(0.0, GetFinalHeight());
    const auto metrics = GetToggleSwitchMetrics();
    const double trackWidth = (std::min)(width, metrics.TrackWidth);
    const double trackHeight = (std::min)(height, metrics.TrackHeight);

    if (track && thumb && trackWidth > 0.0 && trackHeight > 0.0) {
        const double trackY = absoluteY + (height - trackHeight) * 0.5;
        const double thumbInset = (std::min)(metrics.ThumbInset, trackHeight * 0.5);
        const double thumbDiameter = (std::min)(
            trackHeight - 2.0 * thumbInset,
            (std::max)(0.0, trackWidth - 2.0 * thumbInset));
        const double thumbTravel = (std::max)(
            0.0, trackWidth - 2.0 * thumbInset - thumbDiameter);
        const double thumbX = absoluteX + thumbInset + (IsChecked() ? thumbTravel : 0.0);
        const double thumbY = trackY + (trackHeight - thumbDiameter) * 0.5;
        const auto clip = GetClipRect();

        track->SetRect(absoluteX, trackY, trackWidth, trackHeight);
        track->SetCornerRadius(CornerRadius(static_cast<float>(trackHeight * 0.5)));
        track->SetOpacity(GetOpacity());
        track->SetRotation(renderTransform.rotationRadians,
                           renderTransform.pivot.X, renderTransform.pivot.Y);
        track->SetClipRect(clip);
        track->BuildRenderCommands(commands);

        thumb->SetRect(thumbX, thumbY, thumbDiameter, thumbDiameter);
        thumb->SetCornerRadius(CornerRadius(static_cast<float>(thumbDiameter * 0.5)));
        thumb->SetOpacity(GetOpacity());
        thumb->SetRotation(renderTransform.rotationRadians,
                           renderTransform.pivot.X, renderTransform.pivot.Y);
        thumb->SetClipRect(clip);
        thumb->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

void ToggleSwitch::ApplyTheme()
{
    using NeoWPF::Core::ApplicationThemes::GetPaletteBrush;
    using NeoWPF::Core::ApplicationThemes::PaletteBrush;
    using NeoWPF::Core::ApplicationThemes::ThemeFontSize;
    using NeoWPF::Core::ApplicationThemes::ThemeManager;

    if (m_contentTextElement) {
        m_contentTextElement->SetThemePropertyValue(
            "Foreground", GetPaletteBrush(PaletteBrush::Foreground));
        int fontSize = 14;
        if (ThemeManager::Instance().TryGetFontSize(ThemeFontSize::Control, fontSize)) {
            m_contentTextElement->SetThemePropertyValue("FontSize", fontSize);
        }
    }

    if (!m_currentVisualState.empty()) {
        ApplyVisualState(m_currentVisualState, false);
    }
    InvalidateMeasure();
    InvalidateVisual();
}

}