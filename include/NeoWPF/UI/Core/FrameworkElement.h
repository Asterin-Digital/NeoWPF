#pragma once

#include <NeoWPF/UI/Core/UIElement.h>

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

private:
    int m_desiredWidth = 0;
    int m_desiredHeight = 0;
    int m_finalWidth = 0;
    int m_finalHeight = 0;
};

}
