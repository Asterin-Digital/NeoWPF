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
}

void Border::Measure(int availableWidth, int availableHeight)
{
    if (auto presenter = GetPresenter()) {
        presenter->SetPadding(m_padding);

        const int innerWidth = std::max(0, availableWidth - (2 * m_borderThickness) - (2 * m_padding));
        const int innerHeight = std::max(0, availableHeight - (2 * m_borderThickness) - (2 * m_padding));

        presenter->Measure(innerWidth, innerHeight);

        const int desiredW = std::max(0, presenter->GetDesiredWidth() + (2 * m_borderThickness) + (2 * m_padding));
        const int desiredH = std::max(0, presenter->GetDesiredHeight() + (2 * m_borderThickness) + (2 * m_padding));

        SetDesiredSize(desiredW, desiredH);
        return;
    }

    SetDesiredSize(0, 0);
}

void Border::Arrange(int finalWidth, int finalHeight)
{
    FrameworkElement::Arrange(finalWidth, finalHeight);

    if (auto presenter = GetPresenter()) {
        const int innerWidth = std::max(0, finalWidth - (2 * m_borderThickness) - (2 * m_padding));
        const int innerHeight = std::max(0, finalHeight - (2 * m_borderThickness) - (2 * m_padding));

        presenter->Arrange(innerWidth, innerHeight);
        presenter->SetLayoutOffset(m_borderThickness + m_padding, m_borderThickness + m_padding);
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

    if (m_backgroundColor != 0x00000000u) {
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            static_cast<float>(GetFinalWidth()),
            static_cast<float>(GetFinalHeight()),
            m_backgroundColor,
            GetOpacity());
    }

    if (m_borderThickness > 0 && m_borderColor != 0x00000000u) {
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            static_cast<float>(GetFinalWidth()),
            static_cast<float>(m_borderThickness),
            m_borderColor,
            GetOpacity());
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY + GetFinalHeight() - m_borderThickness),
            static_cast<float>(GetFinalWidth()),
            static_cast<float>(m_borderThickness),
            m_borderColor,
            GetOpacity());
        commands.DrawRect(
            static_cast<float>(absX),
            static_cast<float>(absY),
            static_cast<float>(m_borderThickness),
            static_cast<float>(GetFinalHeight()),
            m_borderColor,
            GetOpacity());
        commands.DrawRect(
            static_cast<float>(absX + GetFinalWidth() - m_borderThickness),
            static_cast<float>(absY),
            static_cast<float>(m_borderThickness),
            static_cast<float>(GetFinalHeight()),
            m_borderColor,
            GetOpacity());
    }

    ContentControl::BuildRenderCommands(commands);
}

}
