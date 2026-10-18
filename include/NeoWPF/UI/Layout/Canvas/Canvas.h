/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Layout/Panel/Panel.h>
#include <NeoWPF/UI/Properties/DependencyProperty.h>

namespace NeoWPF::UI {

/*
    @summary
    Positions child elements at explicit coordinates within the panel.

    @details
    Canvas follows WPF placement precedence: Left takes precedence over Right and Top takes precedence over Bottom.
    An unset coordinate is represented by NaN and places the child at the origin when no opposing coordinate is set.
*/
class Canvas : public Panel {
public:
    using Panel::AddChild;

    Canvas() noexcept;

    static const Properties::DependencyProperty& LeftProperty();
    static const Properties::DependencyProperty& TopProperty();
    static const Properties::DependencyProperty& RightProperty();
    static const Properties::DependencyProperty& BottomProperty();

    static double GetLeft(const Visual* element) noexcept;
    static void SetLeft(Visual* element, double value) noexcept;
    static double GetTop(const Visual* element) noexcept;
    static void SetTop(Visual* element, double value) noexcept;
    static double GetRight(const Visual* element) noexcept;
    static void SetRight(Visual* element, double value) noexcept;
    static double GetBottom(const Visual* element) noexcept;
    static void SetBottom(Visual* element, double value) noexcept;

    /*
        @summary
        Measures children without constraining them to the Canvas viewport.

        @param availableWidth
        Width available to the Canvas.

        @param availableHeight
        Height available to the Canvas.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Arranges each child at its Canvas coordinates.

        @param finalWidth
        Width assigned to the Canvas.

        @param finalHeight
        Height assigned to the Canvas.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;
};

}