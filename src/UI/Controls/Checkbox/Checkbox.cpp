#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Controls/Styles.h>
#include <NeoWPF/ControlStyles.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Localization/LocaleOperations.h>
#include <NeoWPF/UI/Properties/Registration/DependencyPropertyRegistration.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <algorithm>
#include <memory>

namespace NeoWPF::UI {

Checkbox::Checkbox()
    : ToggleBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
    SetTemplate(std::make_shared<CheckboxTemplate>());

    APPLY_CONTROL_STYLES(NeoWPF::UI::Controls::Styles, *this);

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    SetPresenter(presenter);

    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    m_textContent->SetVerticalAlignment(VerticalAlignment::Stretch);
    m_textContent->SetText(L"");
    m_textContent->SetTextHorizontalAlignment(TextHorizontalAlignment::Left);
    m_textContent->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    m_textContent->SetTextColor(m_textColor.ToArgb());
    m_textContent->SetFontSize(m_fontSize);
    SetContent(m_textContent);

}

Checkbox::~Checkbox()
{
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

void Checkbox::SetTextColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_textColor = color; if (m_textContent) { m_textContent->SetTextColor(color.ToArgb()); } InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Foreground", *this);
    if (property) { SetValue(*property, color); return; }
    m_textColor = color;
    if (m_textContent) {
        m_textContent->SetTextColor(color.ToArgb());
    }
    InvalidateVisual();
}

void Checkbox::SetBackgroundColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_bgColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Background", *this);
    if (property) { SetValue(*property, color); return; }
    m_bgColor = color;
    InvalidateVisual();
}

void Checkbox::SetBorderColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_borderColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderBrush", *this);
    if (property) { SetValue(*property, color); return; }
    m_borderColor = color;
    InvalidateVisual();
}

void Checkbox::SetPadding(Thickness padding) noexcept
{
    if (IsApplyingPropertyValue()) { m_padding = padding; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Padding", *this);
    if (property) { SetValue(*property, padding); return; }
    m_padding = padding;
    InvalidateVisual();
}

void Checkbox::SetFontSize(int fontSize) noexcept
{
    if (IsApplyingPropertyValue()) { m_fontSize = fontSize; if (m_textContent) { m_textContent->SetFontSize(fontSize); } InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontSize", *this);
    if (property) { SetValue(*property, fontSize); return; }
    m_fontSize = fontSize;
    if (m_textContent) {
        m_textContent->SetFontSize(fontSize);
    }
    InvalidateVisual();
}

void Checkbox::SetFontWeight(int fontWeight) noexcept
{
    if (fontWeight < 100) {
        fontWeight = 100;
    }
    if (fontWeight > 1000) {
        fontWeight = 1000;
    }

    if (IsApplyingPropertyValue()) { m_fontWeight = fontWeight; if (m_textContent) { m_textContent->SetFontWeight(fontWeight); } InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontWeight", *this);
    if (property) { SetValue(*property, fontWeight); return; }
    m_fontWeight = fontWeight;
    if (m_textContent) {
        m_textContent->SetFontWeight(fontWeight);
    }
    InvalidateVisual();
}

int Checkbox::GetFontWeight() const noexcept
{
    return m_fontWeight;
}

void Checkbox::SetFontStyle(NeoWPF::UI::FontStyle fontStyle) noexcept
{
    if (IsApplyingPropertyValue()) { m_fontStyle = fontStyle; if (m_textContent) { m_textContent->SetFontStyle(fontStyle); } InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("FontStyle", *this);
    if (property) { SetValue(*property, fontStyle); return; }
    m_fontStyle = fontStyle;
    if (m_textContent) {
        m_textContent->SetFontStyle(fontStyle);
    }
    InvalidateVisual();
}

NeoWPF::UI::FontStyle Checkbox::GetFontStyle() const noexcept
{
    return m_fontStyle;
}

NeoWPF::UI::Color Checkbox::GetTextColor() const noexcept
{
    return m_textColor;
}

NeoWPF::UI::Color Checkbox::GetBackgroundColor() const noexcept
{
    return m_bgColor;
}

NeoWPF::UI::Color Checkbox::GetBorderColor() const noexcept
{
    return m_borderColor;
}

Thickness Checkbox::GetPadding() const noexcept
{
    return m_padding;
}

int Checkbox::GetFontSize() const noexcept
{
    return m_fontSize;
}

void Checkbox::Measure(double availableWidth, double availableHeight)
{
    const double contentAvailableWidth = availableWidth > 0.0 ? (std::max)(0.0, availableWidth - m_padding.Horizontal()) : 0.0;
    Content::ContentControl::Measure(contentAvailableWidth, 0);

    double contentWidth = 0.0;
    double contentHeight = 0.0;
    if (auto presenter = GetPresenter()) {
        contentWidth = presenter->GetDesiredWidth();
        contentHeight = presenter->GetDesiredHeight();
    }

    const double boxSize = (std::max)(18.0, contentHeight);
    const double desiredWidth = boxSize + m_padding.Horizontal() + contentWidth;
    const double desiredHeight = (std::max)(boxSize, contentHeight) + m_padding.Vertical();

    SetDesiredSize(desiredWidth, desiredHeight);
}

void Checkbox::Arrange(double finalWidth, double finalHeight)
{
    const double contentHeight = (std::max)(0.0, finalHeight);
    Content::ContentControl::Arrange(finalWidth, finalHeight);

    if (auto presenter = GetPresenter()) {
        const double desiredOffsetX = 18.0 + m_padding.Left;
        if (presenter->GetLayoutOffsetX() != desiredOffsetX || presenter->GetLayoutOffsetY() != m_padding.Top) {
            presenter->SetLayoutOffset(desiredOffsetX, m_padding.Top);
        }
        presenter->Arrange((std::max)(0.0, finalWidth - (18.0 + m_padding.Left + m_padding.Right)),
            (std::max)(0.0, contentHeight - m_padding.Top - m_padding.Bottom));
        if (auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(presenter->FindTemplateChild("PART_ContentHost"))) {
            const double desiredTextOffsetY = (finalHeight - contentHeight) / 2.0;
            contentHost->SetRect(
                presenter->GetLayoutOffsetX(),
                desiredTextOffsetY + m_padding.Top,
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
        box->SetBorderThickness(1);
        box->SetOpacity(GetOpacity());
        box->BuildRenderCommands(commands);
    }
    else {
        commands.DrawRoundedRect(boxLeft, boxTop, boxSize, boxSize, 0.0f, 0.0f, 0.0f, 0.0f, m_bgColor.ToArgb(), GetOpacity(), NeoWPF::Rendering::ClipRect{}, renderTransform.rotationRadians, renderTransform.pivot.X, renderTransform.pivot.Y);
        commands.DrawRoundedRect(boxLeft, boxTop, boxSize, boxSize, 0.0f, 0.0f, 0.0f, 0.0f, m_borderColor.ToArgb(), GetOpacity(), NeoWPF::Rendering::ClipRect{}, renderTransform.rotationRadians, renderTransform.pivot.X, renderTransform.pivot.Y);
    }

    if (IsChecked()) {
        const auto checkColor = NeoWPF::UI::Color::FromArgb(0xFF212121u);
        const double checkCenterX = boxLeft + boxSize * 0.5;
        const double checkCenterY = boxTop + boxSize * 0.5;
        constexpr float kCheckAngle = 0.68f;

    auto drawCheckStroke = [&](double width, double height, float rotationRadians, double offsetX, double offsetY) {
        const double x = checkCenterX + offsetX - width * 0.5;
        const double y = checkCenterY + offsetY - height * 0.5;
        commands.DrawRoundedRect(x, y, width, height, 0.0f, 0.0f, 0.0f, 0.0f,
                                checkColor.ToArgb(), GetOpacity(),
                                NeoWPF::Rendering::ClipRect{}, rotationRadians,
                                renderTransform.pivot.X, renderTransform.pivot.Y);
    };

    // left shape
    drawCheckStroke(2.0f, 5.0f, -kCheckAngle, -2.5f, 1.0f);
    // right shape
    drawCheckStroke(8.0f, 1.5f, -kCheckAngle,  1.8f, -1.0f);
}

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const double hostX = fAbsX + boxSize + m_padding.Left;
        const double hostY = fAbsY + m_padding.Top;
        const double hostWidth = (std::max)(0.0, drawWidth - boxSize - m_padding.Left - m_padding.Right);
        const double hostHeight = (std::max)(1.0, drawHeight - m_padding.Top - m_padding.Bottom);
        contentHost->SetRect(
            hostX,
            hostY,
            hostWidth,
            hostHeight);
        contentHost->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

void Checkbox::OnVisualStateApplied(const std::string& stateName)
{
    (void)stateName;
}

}
