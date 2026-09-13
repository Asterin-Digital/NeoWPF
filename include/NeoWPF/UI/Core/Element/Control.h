/*
MIT License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/Color.h>

namespace NeoWPF::UI {

class Control : public FrameworkElement {
public:
    static const Properties::DependencyProperty& PaddingProperty();
    static const Properties::DependencyProperty& BackgroundColorProperty();
    static const Properties::DependencyProperty& BorderColorProperty();
    static const Properties::DependencyProperty& BorderThicknessProperty();

    Control() = default;

    Thickness GetPadding() const noexcept;
    void SetPadding(Thickness padding) noexcept;
    void SetPadding(int left, int top, int right, int bottom) noexcept;

    int GetPaddingLeft() const noexcept;
    int GetPaddingTop() const noexcept;
    int GetPaddingRight() const noexcept;
    int GetPaddingBottom() const noexcept;
    int GetPaddingHorizontal() const noexcept;
    int GetPaddingVertical() const noexcept;

    NeoWPF::UI::Color GetBackgroundColor() const noexcept;
    void SetBackgroundColor(NeoWPF::UI::Color color) noexcept;
    void SetBackgroundColor(std::uint32_t color) noexcept { SetBackgroundColor(NeoWPF::UI::Color::FromArgb(color)); }

    NeoWPF::UI::Color GetBorderColor() const noexcept;
    void SetBorderColor(NeoWPF::UI::Color color) noexcept;
    void SetBorderColor(std::uint32_t color) noexcept { SetBorderColor(NeoWPF::UI::Color::FromArgb(color)); }

    int GetBorderThickness() const noexcept;
    void SetBorderThickness(int thickness) noexcept;

protected:
    virtual void OnPaddingChanged(Thickness padding);
    virtual void OnBackgroundColorChanged(NeoWPF::UI::Color color);
    virtual void OnBorderColorChanged(NeoWPF::UI::Color color);
    virtual void OnBorderThicknessChanged(int thickness);

protected:
    Thickness m_padding{};
    NeoWPF::UI::Color m_backgroundColor = NeoWPF::UI::Color::FromArgb(0xFFFFFFFFu);
    NeoWPF::UI::Color m_borderColor = NeoWPF::UI::Color::FromArgb(0xFFABADB3u);
    int m_borderThickness = 0;
};

}
