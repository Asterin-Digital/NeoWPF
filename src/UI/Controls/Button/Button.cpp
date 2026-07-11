#include <NeoWPF/UI/Controls/Button/Button.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
#include <NeoWPF/Rendering/Renderer.h>
#include <NeoWPF/Rendering/TextRenderer.h>
#include <algorithm>
#include <memory>

#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

Button::Button()
    : ButtonBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
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

    if (auto style = Styling::StyleManager::GetDefault(typeid(Button))) {
        style->Apply(this);
    }
    ApplyVisualState("Normal", false);
}

Button::Button(const std::wstring& text)
    : ButtonBase()
{
    SetHorizontalAlignment(HorizontalAlignment::Left);
    SetVerticalAlignment(VerticalAlignment::Top);
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

    if (auto style = Styling::StyleManager::GetDefault(typeid(Button))) {
        style->Apply(this);
    }

    ApplyVisualState("Normal", false);

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
    int absX, absY;
    GetAbsolutePosition(absX, absY);

    float fAbsX = static_cast<float>(absX);
    float fAbsY = static_cast<float>(absY);
    float drawWidth = static_cast<float>(GetFinalWidth());
    float drawHeight = static_cast<float>(GetFinalHeight());
    ApplyRenderTransform(fAbsX, fAbsY, drawWidth, drawHeight);

    auto background = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Background"));
    if (background) {
        background->SetRect(static_cast<int>(fAbsX), static_cast<int>(fAbsY), static_cast<int>(drawWidth), static_cast<int>(drawHeight));
        background->SetFillColor(m_backgroundColor.ToArgb());
        background->SetBorderColor(0xFF333333u);
        background->SetBorderThickness(1);
        background->SetCornerRadius(GetCornerRadius());
        background->SetOpacity(GetOpacity());
        background->BuildRenderCommands(commands);
    }
    else {
        const auto cornerRadius = GetCornerRadius();
        commands.DrawRoundedRect(
            fAbsX,
            fAbsY,
            drawWidth,
            drawHeight,
            cornerRadius.TopLeft,
            cornerRadius.TopRight,
            cornerRadius.BottomRight,
            cornerRadius.BottomLeft,
            m_backgroundColor.ToArgb(),
            GetOpacity());

        commands.DrawRoundedRect(
            fAbsX,
            fAbsY,
            drawWidth,
            drawHeight,
            cornerRadius.TopLeft,
            cornerRadius.TopRight,
            cornerRadius.BottomRight,
            cornerRadius.BottomLeft,
            0xFF333333u,
            GetOpacity());
    }

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        const int inset = std::max(0, m_padding);
        contentHost->SetRect(static_cast<int>(fAbsX) + inset, static_cast<int>(fAbsY) + inset, std::max(0, static_cast<int>(drawWidth) - (inset * 2)), std::max(0, static_cast<int>(drawHeight) - (inset * 2)));
        contentHost->SetFillColor(0x00000000u);
        contentHost->SetBorderColor(0x00000000u);
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHost->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

void Button::OnVisualStateApplied(const std::string& stateName)
{
    // If developer explicitly set a local BackgroundColor, do not override it with visual states.
    if (IsPropertyLocal("BackgroundColor")) {
        return;
    }

    uint32_t resolvedColor = 0xFF3A3A3Au;
    std::string propertyName = "VisualState." + stateName + ".Background";
    if (!TryGetProperty(propertyName, resolvedColor)) {
        if (stateName == "Normal") {
            resolvedColor = 0xFF3A3A3Au;
        }
    }

    SetBackgroundColor(resolvedColor);
}

}
