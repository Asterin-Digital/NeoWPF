#pragma once

#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Alignment/Alignment.h>

namespace NeoWPF::UI {

class FrameworkElement : public UIElement {
public:
    int GetDesiredWidth() const noexcept { return m_desiredWidth; }
    int GetDesiredHeight() const noexcept { return m_desiredHeight; }

    void SetDesiredSize(int width, int height) noexcept {
        m_desiredWidth = width;
        m_desiredHeight = height;
    }

    void Measure(int availableWidth, int availableHeight) override {
        m_desiredWidth = availableWidth;
        m_desiredHeight = availableHeight;
    }

    void Arrange(int finalWidth, int finalHeight) override {
        m_finalWidth = finalWidth;
        m_finalHeight = finalHeight;
    }

    int GetFinalWidth() const noexcept { return m_finalWidth; }
    int GetFinalHeight() const noexcept { return m_finalHeight; }

    void SetLayoutOffset(int x, int y) noexcept { m_offsetX = x; m_offsetY = y; }
    int GetLayoutOffsetX() const noexcept { return m_offsetX; }
    int GetLayoutOffsetY() const noexcept { return m_offsetY; }

    HorizontalAlignment GetHorizontalAlignment() const noexcept { return m_horizontalAlignment; }
    void SetHorizontalAlignment(HorizontalAlignment align) noexcept { m_horizontalAlignment = align; }

    VerticalAlignment GetVerticalAlignment() const noexcept { return m_verticalAlignment; }
    void SetVerticalAlignment(VerticalAlignment align) noexcept { m_verticalAlignment = align; }

    void GetAbsolutePosition(int& outX, int& outY) const noexcept;

private:
    int m_desiredWidth = 0;
    int m_desiredHeight = 0;
    int m_finalWidth = 0;
    int m_finalHeight = 0;
    int m_offsetX = 0;
    int m_offsetY = 0;
    HorizontalAlignment m_horizontalAlignment = HorizontalAlignment::Stretch;
    VerticalAlignment m_verticalAlignment = VerticalAlignment::Stretch;
};

}
