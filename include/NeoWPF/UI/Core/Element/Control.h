/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/FrameworkElement.h>
#include <NeoWPF/UI/Core/Media/Brush.h>

#include <cstddef>
#include <memory>

namespace NeoWPF::UI {

class Control : public FrameworkElement {
public:
    static const Properties::DependencyProperty& PaddingProperty();
    static const Properties::DependencyProperty& BackgroundProperty();
    static const Properties::DependencyProperty& BorderBrushProperty();
    static const Properties::DependencyProperty& BorderThicknessProperty();

    Control() = default;
    ~Control() override;

    Thickness GetPadding() const noexcept;
    void SetPadding(Thickness padding) noexcept;
    void SetPadding(int left, int top, int right, int bottom) noexcept;

    double GetPaddingLeft() const noexcept;
    double GetPaddingTop() const noexcept;
    double GetPaddingRight() const noexcept;
    double GetPaddingBottom() const noexcept;
    double GetPaddingHorizontal() const noexcept;
    double GetPaddingVertical() const noexcept;

    std::shared_ptr<Brush> GetBackgroundBrush() const;
    void SetBackgroundBrush(std::shared_ptr<Brush> brush);

    std::shared_ptr<Brush> GetBorderBrush() const;
    void SetBorderBrush(std::shared_ptr<Brush> brush);

    double GetBorderThickness() const noexcept;
    void SetBorderThickness(double thickness) noexcept;

protected:
    virtual void OnPaddingChanged(Thickness padding);
    virtual void OnBackgroundBrushChanged(const std::shared_ptr<Brush>& brush);
    virtual void OnBorderBrushChanged(const std::shared_ptr<Brush>& brush);

    virtual void OnBorderThicknessChanged(double thickness);

protected:
    Thickness m_padding{};
    std::weak_ptr<Brush> m_subscribedBackgroundBrush;
    std::weak_ptr<Brush> m_subscribedBorderBrush;
    std::size_t m_backgroundBrushChangedHandlerId = 0;
    std::size_t m_borderBrushChangedHandlerId = 0;
    double m_borderThickness = 0.0;
};

}
