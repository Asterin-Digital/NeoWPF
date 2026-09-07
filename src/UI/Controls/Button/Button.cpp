#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Controls/ButtonStyles.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>
#include <NeoWPF/Diagnostics/Logger.h>
#include <algorithm>
#include <memory>
#include <cmath>

#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

Button::Button()
    : ButtonBase()
{
    /* @note 
    * Keep this as is, in WPF the button defaults to center
    * which matches the intended behavior we want
    */
    SetHorizontalAlignment(HorizontalAlignment::Center);
    SetVerticalAlignment(VerticalAlignment::Center);
    SetTemplate(std::make_shared<ButtonTemplate>());

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(8);
    SetPresenter(presenter);

    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    m_textContent->SetVerticalAlignment(VerticalAlignment::Stretch);
    m_textContent->SetText(L"");
    m_textContent->SetTextHorizontalAlignment(TextHorizontalAlignment::Center);
    m_textContent->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    m_textContent->SetTextColor(m_textColor.ToArgb());
    m_textContent->SetFontSize(m_fontSize);
    SetContent(m_textContent);

    NeoWPF::UI::Generated::ButtonStyles styles;
    styles.InitializeComponent();
    NEOWPF_APPLY_STYLES(styles, *this);

    SetPadding(8);
    SetBackgroundColor(m_backgroundColor);
    SetBorderColor(m_borderColor);
}

Button::Button(const std::wstring& text)
    : ButtonBase()
{
    /* @note 
    * Keep this as is, in WPF the button defaults to center
    * which matches the intended behavior we want
    */
    SetHorizontalAlignment(HorizontalAlignment::Center);
    SetVerticalAlignment(VerticalAlignment::Center);
    SetTemplate(std::make_shared<ButtonTemplate>());

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(8);
    SetPresenter(presenter);

    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetHorizontalAlignment(HorizontalAlignment::Stretch);
    m_textContent->SetVerticalAlignment(VerticalAlignment::Stretch);
    m_textContent->SetText(L"");
    m_textContent->SetTextHorizontalAlignment(TextHorizontalAlignment::Center);
    m_textContent->SetTextVerticalAlignment(TextVerticalAlignment::Center);
    m_textContent->SetTextColor(m_textColor.ToArgb());
    m_textContent->SetFontSize(m_fontSize);
    SetContent(m_textContent);

    NeoWPF::UI::Generated::ButtonStyles styles;
    styles.InitializeComponent();
    NEOWPF_APPLY_STYLES(styles, *this);

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
    if (!IsApplyingPropertyValue()) {
        const auto* property = Properties::DependencyPropertyRegistry::Get().FindProperty("Text", *this);
        if (property) {
            SetValue(*property, text);
            return;
        }
    }

    m_text = text;
    if (m_textContent) {
        m_textContent->SetText(text);
    }
    InvalidateVisual();
}

void Button::SetTextHorizontalAlignment(TextHorizontalAlignment alignment) noexcept
{
    if (m_textContent) {
        m_textContent->SetTextHorizontalAlignment(alignment);
    }
    InvalidateVisual();
}

TextHorizontalAlignment Button::GetTextHorizontalAlignment() const noexcept
{
    if (m_textContent) {
        return m_textContent->GetTextHorizontalAlignment();
    }
    return TextHorizontalAlignment::Left;
}

void Button::SetTextVerticalAlignment(TextVerticalAlignment alignment) noexcept
{
    if (m_textContent) {
        m_textContent->SetTextVerticalAlignment(alignment);
    }
    InvalidateVisual();
}

TextVerticalAlignment Button::GetTextVerticalAlignment() const noexcept
{
    if (m_textContent) {
        return m_textContent->GetTextVerticalAlignment();
    }
    return TextVerticalAlignment::Top;
}

void Button::SetPadding(Thickness padding) noexcept
{
    Control::SetPadding(padding);
    InvalidateVisual();
}

Thickness Button::GetPadding() const noexcept
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

void Button::OnPaddingChanged(Thickness padding)
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
    float rotationRadians = GetWorldRotationRadians();

    const auto cornerRadius = GetCornerRadius();

    auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"));
    if (background) {
        background->SetRect(
            static_cast<int>(std::lround(drawX)),
            static_cast<int>(std::lround(drawY)),
            std::max(0, static_cast<int>(std::lround(drawWidth))),
            std::max(0, static_cast<int>(std::lround(drawHeight))));
        background->SetBorderThickness(1);
        background->SetCornerRadius(cornerRadius);
        background->SetOpacity(GetOpacity());
        background->SetRotation(rotationRadians);
        background->BuildRenderCommands(commands);
    }
    else {
        const float borderThickness = 1.0f;
        const uint32_t borderColor = 0xFF707070u;
        const float borderInset = borderThickness * 0.5f;
        const float borderX = drawX + borderInset;
        const float borderY = drawY + borderInset;
        const float borderW = std::max(0.0f, drawWidth - borderThickness);
        const float borderH = std::max(0.0f, drawHeight - borderThickness);

        commands.DrawRoundedRect(
            borderX,
            borderY,
            borderW,
            borderH,
            std::max(0.0f, cornerRadius.TopLeft - borderInset),
            std::max(0.0f, cornerRadius.TopRight - borderInset),
            std::max(0.0f, cornerRadius.BottomRight - borderInset),
            std::max(0.0f, cornerRadius.BottomLeft - borderInset),
            borderColor,
            GetOpacity(),
            NeoWPF::Rendering::ClipRect{},
            rotationRadians);

        const float inset = borderThickness;
        const float innerX = drawX + inset;
        const float innerY = drawY + inset;
        const float innerW = std::max(0.0f, drawWidth - 2.0f * inset);
        const float innerH = std::max(0.0f, drawHeight - 2.0f * inset);
        if (innerW > 0.0f && innerH > 0.0f) {
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
        const auto padding = GetPadding();
        const int insetX = std::max(0, padding.Left);
        const int insetY = std::max(0, padding.Top);
        const int chX = static_cast<int>(std::lround(drawX)) + insetX;
        const int chY = static_cast<int>(std::lround(drawY)) + insetY;
        const int chW = std::max(0, static_cast<int>(std::lround(drawWidth)) - padding.Horizontal());
        const int chH = std::max(0, static_cast<int>(std::lround(drawHeight)) - padding.Vertical());
        NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Info,
            "[Button] BuildRenderCommands: abs=(%d,%d) size=(%d,%d) inset=(%d,%d) contentHost=(%d,%d,%d,%d) contentHostPtr=%p",
            absX, absY, GetFinalWidth(), GetFinalHeight(), insetX, insetY, chX, chY, chW, chH, contentHost.get());

        contentHost->SetRect(chX, chY, chW, chH);
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHost->SetRotation(rotationRadians);
        contentHost->BuildRenderCommands(commands);

        auto presenter = GetPresenter();
        if (presenter) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
                "[Button] Presenter: ptr=%p final=(%d,%d) layoutOffset=(%d,%d)",
                presenter.get(), presenter->GetFinalWidth(), presenter->GetFinalHeight(), presenter->GetLayoutOffsetX(), presenter->GetLayoutOffsetY());
        }

        if (m_textContent) {
            NeoWPF::Diagnostics::Log(NeoWPF::Diagnostics::Category::Control, NeoWPF::Diagnostics::Level::Debug,
                "[Button] TextContent: ptr=%p final=(%d,%d) layoutOffset=(%d,%d)",
                m_textContent.get(), m_textContent->GetFinalWidth(), m_textContent->GetFinalHeight(), m_textContent->GetLayoutOffsetX(), m_textContent->GetLayoutOffsetY());
        }
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

}