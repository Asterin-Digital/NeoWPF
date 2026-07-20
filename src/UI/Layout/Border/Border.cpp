#include <NeoWPF/UI/Layout/Border/Border.h>
#include <NeoWPF/UI/Content/ContentPresenter.h>
#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <algorithm>

namespace NeoWPF::UI {

Border::Border()
{
    auto presenter = std::make_shared<NeoWPF::UI::Content::ContentPresenter>();
    presenter->SetPadding(m_padding);
    SetPresenter(presenter);

    ApplyBorderThemeStyle(this);
}

void Border::Measure(int availableWidth, int availableHeight)
{
    if (auto presenter = GetPresenter()) {
        if (!presenter->GetParent()) {
            try {
                AddChild(presenter);
            } catch (const std::bad_weak_ptr&) {
            }
        }

        if (presenter->GetPadding() != m_padding) {
            presenter->SetPadding(m_padding);
        }

        const int explicitW = GetWidth();
        const int explicitH = GetHeight();
        const int availableContentWidth = std::max(0, explicitW > 0 ? explicitW : availableWidth);
        const int availableContentHeight = std::max(0, explicitH > 0 ? explicitH : availableHeight);

        const int innerWidth = std::max(0, availableContentWidth - (2 * m_borderThickness) - (2 * m_padding));
        const int innerHeight = std::max(0, availableContentHeight - (2 * m_borderThickness) - (2 * m_padding));

        presenter->Measure(innerWidth, innerHeight);

        const int contentW = presenter->GetDesiredWidth();
        const int contentH = presenter->GetDesiredHeight();
        const int contentDesiredW = std::max(0, contentW + (2 * m_borderThickness) + (2 * m_padding));
        const int contentDesiredH = std::max(0, contentH + (2 * m_borderThickness) + (2 * m_padding));

        const int desiredW = std::max(contentDesiredW, explicitW > 0 ? explicitW : 0);
        const int desiredH = std::max(contentDesiredH, explicitH > 0 ? explicitH : 0);

        SetDesiredSize(desiredW, desiredH);
        return;
    }

    SetDesiredSize(0, 0);
}

void Border::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    if (auto presenter = GetPresenter()) {
        const int innerX = m_borderThickness + m_padding;
        const int innerY = m_borderThickness + m_padding;
        const int innerWidth = std::max(0, finalWidth - (2 * m_borderThickness) - (2 * m_padding));
        const int innerHeight = std::max(0, finalHeight - (2 * m_borderThickness) - (2 * m_padding));

        presenter->Arrange(innerWidth, innerHeight);
        presenter->SetLayoutOffset(innerX, innerY);
        /* if (!presenter->GetParent()) {
            try {
                AddChild(presenter);
            } catch (const std::bad_weak_ptr&) {
            }
        } */ // possible crash point
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

    if (m_borderThickness > 0 && m_borderColor.ToArgb() != 0x00000000u) {
        commands.DrawRoundedRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            static_cast<float>(GetFinalWidth()),
            static_cast<float>(GetFinalHeight()),
            cornerRadius.TopLeft,
            cornerRadius.TopRight,
            cornerRadius.BottomRight,
            cornerRadius.BottomLeft,
            m_borderColor.ToArgb(),
            GetOpacity());
    }

    if (m_backgroundColor.ToArgb() != 0x00000000u) {
        const float inset = static_cast<float>(m_borderThickness);
        commands.DrawRoundedRect(
            static_cast<float>(absX + inset),
            static_cast<float>(absY + inset),
            static_cast<float>(std::max(0, GetFinalWidth() - (2 * m_borderThickness))),
            static_cast<float>(std::max(0, GetFinalHeight() - (2 * m_borderThickness))),
            std::max(0.0f, cornerRadius.TopLeft - inset),
            std::max(0.0f, cornerRadius.TopRight - inset),
            std::max(0.0f, cornerRadius.BottomRight - inset),
            std::max(0.0f, cornerRadius.BottomLeft - inset),
            m_backgroundColor.ToArgb(),
            GetOpacity());
    }

    ContentControl::BuildRenderCommands(commands);
}

}
