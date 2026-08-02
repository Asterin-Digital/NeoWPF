#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/Color.h>

namespace NeoWPF::UI {

class Control : public FrameworkElement {
public:
    Control() = default;

    int GetPadding() const noexcept;
    void SetPadding(int padding) noexcept;

    NeoWPF::UI::Color GetBackgroundColor() const noexcept;
    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept;
    void SetBackgroundColor(std::uint32_t color) noexcept { SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color)); }

    NeoWPF::UI::Color GetBorderColor() const noexcept;
    void SetBorderColor(NeoWPF::UI::Color color) noexcept;
    void SetBorderColor(std::uint32_t color) noexcept { SetBorderColor(NeoWPF::UI::Color::FromArgb(color)); }

    int GetBorderThickness() const noexcept;
    void SetBorderThickness(int thickness) noexcept;

protected:
    virtual void OnPaddingChanged(int padding);
    virtual void OnBackgroundColorChanged(NeoWPF::UI::Color color);
    virtual void OnBorderColorChanged(NeoWPF::UI::Color color);
    virtual void OnBorderThicknessChanged(int thickness);

protected:
    int m_padding = 0;
    NeoWPF::UI::Color m_backgroundColor = NeoWPF::UI::Color::FromArgb(0x00000000u);
    NeoWPF::UI::Color m_borderColor = NeoWPF::UI::Color::FromArgb(0x00000000u);
    int m_borderThickness = 0;
};

}
