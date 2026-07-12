#include <NeoWPF/UI/Controls/Checkbox/Checkbox.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Controls/Templates/TemplateVisual.h>
#include <NeoWPF/UI/Controls/Text/TextBlock.h>
#include <NeoWPF/UI/Styling/Style/Style.h>
#include <NeoWPF/UI/Styling/Style/StyleManager.h>
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
    ApplyTemplate();

    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(8);
    SetPresenter(presenter);

    m_textContent = std::make_shared<NeoWPF::UI::TextBlock>();
    m_textContent->SetText(L"");
    m_textContent->SetTextColor(m_textColor.ToArgb());
    m_textContent->SetFontSize(14);
    SetContent(m_textContent);

    if (auto style = Styling::StyleManager::GetDefault(typeid(Checkbox))) {
        style->Apply(this);
    }

    ApplyVisualState("Normal", false);
}

Checkbox::~Checkbox()
{
}

void Checkbox::SetText(const std::wstring& text)
{
    m_text = text;
    if (m_textContent) {
        m_textContent->SetText(text);
    }
    InvalidateVisual();
}

void Checkbox::Measure(int availableWidth, int availableHeight)
{
    Content::ContentControl::Measure(availableWidth, availableHeight);
}

void Checkbox::Arrange(int finalWidth, int finalHeight)
{
    Content::ContentControl::Arrange(finalWidth, finalHeight);
}

void Checkbox::BuildRenderCommands(NeoWPF::Rendering::CommandList& commands)
{
    int absX, absY;
    GetAbsolutePosition(absX, absY);

    float fAbsX = static_cast<float>(absX);
    float fAbsY = static_cast<float>(absY);
    float drawWidth = static_cast<float>(GetFinalWidth());
    float drawHeight = static_cast<float>(GetFinalHeight());
    ApplyRenderTransform(fAbsX, fAbsY, drawWidth, drawHeight);

    auto box = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_Box"));
    if (box) {
        box->SetRect(static_cast<int>(fAbsX), static_cast<int>(fAbsY), 18, 18);
        box->SetFillColor(m_boxColor.ToArgb());
        box->SetBorderColor(m_borderColor.ToArgb());
        box->SetBorderThickness(1);
        box->SetOpacity(GetOpacity());
        box->BuildRenderCommands(commands);
    }
    else {
        commands.DrawRoundedRect(fAbsX, fAbsY, 18.0f, 18.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_boxColor.ToArgb(), GetOpacity());
        commands.DrawRoundedRect(fAbsX, fAbsY, 18.0f, 18.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_borderColor.ToArgb(), GetOpacity());
    }

    if (IsChecked()) {
        const auto checkColor = NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu);
        commands.DrawRect(fAbsX + 4.0f, fAbsY + 8.0f, 3.0f, 6.0f, checkColor.ToArgb(), GetOpacity());
        commands.DrawRect(fAbsX + 7.0f, fAbsY + 5.0f, 8.0f, 3.0f, checkColor.ToArgb(), GetOpacity());
    }

    auto contentHost = std::dynamic_pointer_cast<Templates::TemplateVisual>(FindTemplateChild("PART_ContentHost"));
    if (contentHost) {
        contentHost->SetRect(static_cast<int>(fAbsX) + 24, static_cast<int>(fAbsY), std::max(0, static_cast<int>(drawWidth) - 24), static_cast<int>(drawHeight));
        contentHost->SetBorderThickness(0);
        contentHost->SetOpacity(GetOpacity());
        contentHost->BuildRenderCommands(commands);
    }

    Content::ContentControl::BuildRenderCommands(commands);
}

void Checkbox::OnVisualStateApplied(const std::string& stateName)
{
    (void)stateName;
}

}
