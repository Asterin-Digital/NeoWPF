#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <algorithm>
#include <memory>
#include <cmath>

#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

namespace {

float GetAbsoluteRenderTransformRotation(const UIElement* element)
{
    using namespace NeoWPF::UI::Core;
    Matrix4 matrix = Matrix4::Identity();
    const UIElement* current = element;
    while (current) {
        if (auto fe = dynamic_cast<const FrameworkElement*>(current)) {
            matrix = matrix * Matrix4::Translation(
                static_cast<float>(fe->GetLayoutOffsetX() + fe->GetRenderOffsetX()),
                static_cast<float>(fe->GetLayoutOffsetY() + fe->GetRenderOffsetY()));
        }

        if (auto transform = current->GetRenderTransform()) {
            float width = 0.0f;
            float height = 0.0f;
            if (auto fe = dynamic_cast<const FrameworkElement*>(current)) {
                width = static_cast<float>((std::max)(fe->GetFinalWidth(), fe->GetDesiredWidth()));
                height = static_cast<float>((std::max)(fe->GetFinalHeight(), fe->GetDesiredHeight()));
            }

            const auto origin = current->GetRenderTransformOrigin();
            const float originX = width * origin.X;
            const float originY = height * origin.Y;

            if (originX != 0.0f || originY != 0.0f) {
                matrix = matrix * Matrix4::Translation(originX, originY);
                matrix = matrix * transform->GetMatrix();
                matrix = matrix * Matrix4::Translation(-originX, -originY);
            } else {
                matrix = matrix * transform->GetMatrix();
            }
        }

        current = dynamic_cast<const UIElement*>(current->GetParent());
    }

    return std::atan2(matrix.m[1][0], matrix.m[0][0]);
}

}

Button::Button()
    : ButtonBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Stretch);
    SetVerticalAlignment(VerticalAlignment::Stretch);
    SetTemplate(std::make_shared<ButtonTemplate>());
    ApplyTemplate();

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(8);
    SetPresenter(presenter);

    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetText(L"");
    m_textContent->SetTextColor(m_textColor.ToArgb());
    m_textContent->SetFontSize(m_fontSize);
    SetContent(m_textContent);

    ApplyButtonThemeStyle(this);
    ApplyVisualState("Normal", false);

    SetPadding(8);
    SetBackgroundColor(m_backgroundColor);
    SetBorderColor(m_borderColor);
}

Button::Button(const std::wstring& text)
    : ButtonBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Stretch);
    SetVerticalAlignment(VerticalAlignment::Stretch);
    SetTemplate(std::make_shared<ButtonTemplate>());
    ApplyTemplate();

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(8);
    SetPresenter(presenter);

    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetText(L"");
    m_textContent->SetTextColor(m_textColor.ToArgb());
    m_textContent->SetFontSize(m_fontSize);
    SetContent(m_textContent);

    ApplyButtonThemeStyle(this);

    ApplyVisualState("Normal", false);

    SetPadding(8);
    SetBackgroundColor(m_backgroundColor);
    SetBorderColor(m_borderColor);
    SetText(text);
}

Button::~Button()
{
}

void Button::SetText(const std::wstring& text)
{
    m_text = text;
    if (m_textContent) {
        m_textContent->SetText(text);
    }
    InvalidateVisual();
}

void Button::SetPadding(int padding) noexcept
{
    Control::SetPadding(padding);
    InvalidateVisual();
}

int Button::GetPadding() const noexcept
{
    return Control::GetPadding();
}

void Button::SetBackgroundColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBackgroundColor(color);
    InvalidateVisual();
}

void Button::SetBorderColor(NeoWPF::UI::Color color) noexcept
{
    Control::SetBorderColor(color);
    InvalidateVisual();
}

NeoWPF::UI::Color Button::GetBorderColor() const noexcept
{
    return m_borderColor;
}

void Button::OnPaddingChanged(int padding)
{
    if (auto presenter = GetPresenter()) {
        presenter->SetPadding(padding);
    }
    InvalidateVisual();
}

void Button::OnBackgroundColorChanged(NeoWPF::UI::Color color)
{
    m_baseBackgroundColor = color;
    m_backgroundColor = color;

    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetFillColor(color.ToArgb());
    }

    InvalidateVisual();
}

void Button::OnBorderColorChanged(NeoWPF::UI::Color color)
{
    m_borderColor = color;

    if (auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"))) {
        background->SetBorderColor(color.ToArgb());
    }

    InvalidateVisual();
}

void Button::OnBorderThicknessChanged(int)
{
    InvalidateVisual();
}

void Button::Measure(int availableWidth, int availableHeight)
{
    Content::ContentControl::Measure(availableWidth, availableHeight);
}

void Button::Arrange(int finalWidth, int finalHeight)
{
    Content::ContentControl::Arrange(finalWidth, finalHeight);
}

void Button::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX = 0;
    int absY = 0;
    GetAbsolutePosition(absX, absY);

    float drawX = static_cast<float>(absX);
    float drawY = static_cast<float>(absY);
    float drawWidth = static_cast<float>(GetFinalWidth());
    float drawHeight = static_cast<float>(GetFinalHeight());
    ApplyRenderTransform(drawX, drawY, drawWidth, drawHeight);
    float rotationRadians = GetAbsoluteRenderTransformRotation(this);

    const auto cornerRadius = GetCornerRadius();

    auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"));
    if (background) {
        background->SetRect(static_cast<int>(std::lround(drawX)), static_cast<int>(std::lround(drawY)), static_cast<int>(drawWidth), static_cast<int>(drawHeight));
        background->SetBorderThickness(1);
        background->SetCornerRadius(cornerRadius);
        background->SetOpacity(GetOpacity());
        background->SetRotation(rotationRadians);
        background->BuildRenderCommands(commands);
    }
    else {
        const float borderThickness = 1.0f; 
        const uint32_t borderColor = 0xFF707070u; 

        commands.DrawRoundedRect(
            drawX,
            drawY,
            drawWidth,
            drawHeight,
            cornerRadius.TopLeft,
            cornerRadius.TopRight,
            cornerRadius.BottomRight,
            cornerRadius.BottomLeft,
            borderColor,
            GetOpacity(),
            NeoWPF::Rendering::ClipRect{},
            rotationRadians);

        // Draw the background inset by border thickness
        const float inset = borderThickness;
        const float innerX = drawX + inset;
        const float innerY = drawY + inset;
        const float innerW = drawWidth - 2.0f * inset;
        const float innerH = drawHeight - 2.0f * inset;
        if (innerW > 0.0f && innerH > 0.0f) {
            // Adjust corner radius for the inner rectangle (subtract inset, but not below 0)
            const float innerTL = std::max(0.0f, cornerRadius.TopLeft - inset);
            const float innerTR = std::max(0.0f, cornerRadius.TopRight - inset);
            const float innerBR = std::max(0.0f, cornerRadius.BottomRight - inset);
            const float innerBL = std::max(0.0f, cornerRadius.BottomLeft - inset);

            commands.DrawRoundedRect(
                innerX,
                innerY,
                innerW,
                innerH,
                innerTL,
                innerTR,
                innerBR,
                innerBL,
                m_backgroundColor.ToArgb(),
                GetOpacity(),
                NeoWPF::Rendering::ClipRect{},
                rotationRadians);
        }
    }

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const int inset = std::max(0, GetPadding());
        contentHost->SetRect(static_cast<int>(std::lround(drawX)) + inset, static_cast<int>(std::lround(drawY)) + inset, std::max(0, static_cast<int>(drawWidth) - (inset * 2)), std::max(0, static_cast<int>(drawHeight) - (inset * 2)));
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHost->SetRotation(rotationRadians);
        contentHost->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

}