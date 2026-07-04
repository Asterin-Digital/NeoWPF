#pragma once

#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Layout/Alignment/Alignment.h>

namespace NeoWPF::UI {

enum class LayoutSizeMode {
    Auto,
    Stretch,
    Explicit
};

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

    LayoutSizeMode GetWidthMode() const noexcept { return m_widthMode; }
    void SetWidthMode(LayoutSizeMode mode) noexcept { m_widthMode = mode; InvalidateVisual(); }

    LayoutSizeMode GetHeightMode() const noexcept { return m_heightMode; }
    void SetHeightMode(LayoutSizeMode mode) noexcept { m_heightMode = mode; InvalidateVisual(); }

    void SetWidth(int width) noexcept { m_widthMode = LayoutSizeMode::Explicit; m_explicitWidth = width < 0 ? 0 : width; InvalidateVisual(); }
    void SetHeight(int height) noexcept { m_heightMode = LayoutSizeMode::Explicit; m_explicitHeight = height < 0 ? 0 : height; InvalidateVisual(); }
    void ClearWidth() noexcept { m_widthMode = LayoutSizeMode::Auto; m_explicitWidth = 0; InvalidateVisual(); }
    void ClearHeight() noexcept { m_heightMode = LayoutSizeMode::Auto; m_explicitHeight = 0; InvalidateVisual(); }

    int ResolveWidth(int availableWidth, int desiredWidth) const;
    int ResolveHeight(int availableHeight, int desiredHeight) const;

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
    LayoutSizeMode m_widthMode = LayoutSizeMode::Auto;
    LayoutSizeMode m_heightMode = LayoutSizeMode::Auto;
    int m_explicitWidth = 0;
    int m_explicitHeight = 0;
    HorizontalAlignment m_horizontalAlignment = HorizontalAlignment::Stretch;
    VerticalAlignment m_verticalAlignment = VerticalAlignment::Stretch;
};

}
