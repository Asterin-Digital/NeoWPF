/*
Apache 2.0 License — Copyright (c) 2026 NeoWPF
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Content/ContentControl.h>

namespace NeoWPF::UI {

class FrameworkElement;
class Window;

enum class PopupPlacement {
    Bottom,
    Top,
    Left,
    Right
};

class Popup : public Content::ContentControl {
public:
    static const Properties::DependencyProperty& IsOpenProperty();
    static const Properties::DependencyProperty& StaysOpenProperty();
    static const Properties::DependencyProperty& PlacementTargetProperty();
    static const Properties::DependencyProperty& PlacementProperty();
    static const Properties::DependencyProperty& HorizontalOffsetProperty();
    static const Properties::DependencyProperty& VerticalOffsetProperty();

    Popup();
    ~Popup() override;

    bool GetIsOpen() const noexcept;
    void SetIsOpen(bool isOpen);

    bool GetStaysOpen() const noexcept;
    void SetStaysOpen(bool staysOpen);

    FrameworkElement* GetPlacementTarget() const noexcept;
    void SetPlacementTarget(FrameworkElement* target);

    PopupPlacement GetPlacement() const noexcept;
    void SetPlacement(PopupPlacement placement);

    double GetHorizontalOffset() const noexcept;
    void SetHorizontalOffset(double offset);

    double GetVerticalOffset() const noexcept;
    void SetVerticalOffset(double offset);

    bool IsHitTestVisible() const noexcept override;
    void Measure(double availableWidth, double availableHeight) override;
    void Arrange(double finalWidth, double finalHeight) override;
    void BuildRenderCommands(NeoWPF::Rendering::CommandList& commands) override;

private:
    Window* m_hostWindow = nullptr;
};

}
