#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
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

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    SetPresenter(presenter);

    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    m_textContent->SetVerticalAlignment(VerticalAlignment::Stretch);
    m_textContent->SetText(L"");
    m_textContent->SetTextHorizontalAlignment(TextHorizontalAlignment::Left);
    m_textContent->SetTextColor(m_textColor.ToArgb());
    m_textContent->SetFontSize(m_fontSize);
    SetContent(m_textContent);

    ApplyCheckboxThemeStyle(this);
}

Checkbox::~Checkbox()
{
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
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("TextColor", *this);
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
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BackgroundColor", *this);
    if (property) { SetValue(*property, color); return; }
    m_bgColor = color;
    InvalidateVisual();
}

void Checkbox::SetBorderColor(NeoWPF::UI::Color color) noexcept
{
    if (IsApplyingPropertyValue()) { m_borderColor = color; InvalidateVisual(); return; }
    const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("BorderColor", *this);
    if (property) { SetValue(*property, color); return; }
    m_borderColor = color;
    InvalidateVisual();
}

void Checkbox::SetPadding(int padding) noexcept
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

int Checkbox::GetPadding() const noexcept
{
    return m_padding;
}

int Checkbox::GetFontSize() const noexcept
{
    return m_fontSize;
}

void Checkbox::Measure(int availableWidth, int availableHeight)
{
    const int contentAvailableWidth = availableWidth > 0 ? std::max(0, availableWidth - m_padding) : 0;
    Content::ContentControl::Measure(contentAvailableWidth, 0);

    int contentWidth = 0;
    int contentHeight = 0;
    if (auto presenter = GetPresenter()) {
        contentWidth = presenter->GetDesiredWidth();
        contentHeight = presenter->GetDesiredHeight();
    }

    const int boxSize = std::max(18, contentHeight);
    int desiredWidth = boxSize + m_padding + contentWidth;
    int desiredHeight = std::max(boxSize, contentHeight);

    SetDesiredSize(desiredWidth, desiredHeight);
}

void Checkbox::Arrange(int finalWidth, int finalHeight)
{
    const int contentHeight = std::max(0, finalHeight);
    Content::ContentControl::Arrange(finalWidth, finalHeight);

    if (auto presenter = GetPresenter()) {
        const int desiredOffsetX = 18 + m_padding;
        if (presenter->GetLayoutOffsetX() != desiredOffsetX || presenter->GetLayoutOffsetY() != 0) {
            presenter->SetLayoutOffset(desiredOffsetX, 0);
        }
        presenter->Arrange(std::max(0, finalWidth - (18 + m_padding)), contentHeight);
    }
}

void Checkbox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    const int boxSize = 18;

    int absX, absY;
    GetAbsolutePosition(absX, absY);

    float fAbsX = static_cast<float>(absX);
    float fAbsY = static_cast<float>(absY);
    float drawWidth = static_cast<float>(GetFinalWidth());
    float drawHeight = static_cast<float>(GetFinalHeight());
    ApplyRenderTransform(fAbsX, fAbsY, drawWidth, drawHeight);

    auto box = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Box"));
    if (box) {
        box->SetRect(static_cast<int>(std::lround(fAbsX)), static_cast<int>(std::lround(fAbsY + 1)), boxSize, boxSize);
        box->SetFillColor(m_bgColor.ToArgb());
        box->SetBorderColor(m_borderColor.ToArgb());
        box->SetBorderThickness(1);
        box->SetOpacity(GetOpacity());
        box->BuildRenderCommands(commands);
    }
    else {
        commands.DrawRoundedRect(fAbsX, fAbsY + 1.0f, 18.0f, 18.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_bgColor.ToArgb(), GetOpacity());
        commands.DrawRoundedRect(fAbsX, fAbsY + 1.0f, 18.0f, 18.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_borderColor.ToArgb(), GetOpacity());
    }

if (IsChecked()) {
    const auto checkColor = NeoWPF::UI::Color::FromArgb(0xFF212121u);
    const float checkCenterX = fAbsX + 9.0f;
    const float checkCenterY = fAbsY + 10.0f;
    constexpr float kCheckAngle = 0.68f;

    auto drawCheckStroke = [&](float width, float height, float rotationRadians, float offsetX, float offsetY) {
        const float x = checkCenterX + offsetX - width * 0.5f;
        const float y = checkCenterY + offsetY - height * 0.5f;
        commands.DrawRoundedRect(x, y, width, height, 0.0f, 0.0f, 0.0f, 0.0f,
                                checkColor.ToArgb(), GetOpacity(),
                                NeoWPF::Rendering::ClipRect{}, rotationRadians);
    };

    // left shape
    drawCheckStroke(2.0f, 5.0f, -kCheckAngle, -2.5f, 1.0f);
    // right shape
    drawCheckStroke(8.0f, 1.5f, -kCheckAngle,  1.8f, -1.0f);
}

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        contentHost->SetRect(
            static_cast<int>(std::lround(fAbsX)) + boxSize + m_padding,
            static_cast<int>(std::lround(fAbsY)),
            std::max(0, static_cast<int>(std::lround(drawWidth)) - (boxSize + m_padding)),
            std::max(1, static_cast<int>(std::lround(drawHeight))));
        contentHost->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

void Checkbox::OnVisualStateApplied(const std::string& stateName)
{
    (void)stateName;
}

}
