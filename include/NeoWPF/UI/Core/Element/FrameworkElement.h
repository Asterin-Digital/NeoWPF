/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

#pragma once

#include <NeoWPF/UI/Core/Element/UIElement.h>
#include <NeoWPF/UI/Core/Media/CornerRadius.h>
#include <NeoWPF/UI/Core/Media/Spacing.h>
#include <NeoWPF/UI/Core/Media/Thickness.h>
#include <NeoWPF/UI/Layout/Placement/Alignment.h>
#include <algorithm>

namespace NeoWPF::UI::Data {
class INotifyPropertyChanged;
}
#include <any>
#include <memory>
#include <type_traits>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace NeoWPF::UI {

class ScrollBar;

/*
    @summary
    Controls how a framework element resolves its width or height.
*/
enum class LayoutSizeMode {
    Auto,
    Stretch
};

class FrameworkElement : public UIElement {
    friend class ScrollBar;
public:
    static const Properties::DependencyProperty& WidthProperty();
    static const Properties::DependencyProperty& HeightProperty();
    static const Properties::DependencyProperty& WidthModeProperty();
    static const Properties::DependencyProperty& HeightModeProperty();
    static const Properties::DependencyProperty& ClipToBoundsProperty();
    static const Properties::DependencyProperty& HorizontalAlignmentProperty();
    static const Properties::DependencyProperty& VerticalAlignmentProperty();
    static const Properties::DependencyProperty& MarginProperty();
    static const Properties::DependencyProperty& SpacingProperty();
    static const Properties::DependencyProperty& CornerRadiusProperty();
    static const Properties::DependencyProperty& DataContextProperty();

    using UIElement::BeginAnimation;
    /*
        @summary
        Returns the size the element wants during measurement.

        @internal
    */
    double GetDesiredWidth() const noexcept { return m_desiredWidth; }

    /*
        @summary
        Returns the size the element wants during measurement.

        @internal
    */
    double GetDesiredHeight() const noexcept { return m_desiredHeight; }

    /*
        @summary
        Sets the size the element reports during measurement.

        @param width
        The desired width.

        @param height
        The desired height.

        @internal
    */
    void SetDesiredSize(double width, double height) noexcept {
        m_desiredWidth = width;
        m_desiredHeight = height;
    }

    /*
        @summary
        Measures the element using the available space.

        @param availableWidth
        The width available to the element.

        @param availableHeight
        The height available to the element.

        @internal
    */
    void Measure(double availableWidth, double availableHeight) override;

    /*
        @summary
        Places the element in its final bounds.

        @param finalWidth
        The width assigned to the element.

        @param finalHeight
        The height assigned to the element.

        @internal
    */
    void Arrange(double finalWidth, double finalHeight) override;

    NeoWPF::UI::Core::Matrix4 GetLocalPositionMatrix() const noexcept override;

    /*
        @summary
        Returns the width assigned during arrange.

        @internal
    */
    double GetFinalWidth() const noexcept { return m_finalWidth; }

    /*
        @summary
        Returns the height assigned during arrange.

        @internal
    */
    double GetFinalHeight() const noexcept { return m_finalHeight; }

    /*
        @summary
        Returns whether the element clips its rendered content to its own bounds.
    */
    bool IsClipToBounds() const noexcept;

    /*
        @summary
        Enables or disables clipping of the element's rendered content to its own bounds.
    */
    void SetClipToBounds(bool clipToBounds) noexcept;

    /*
        @summary
        Returns the clip rectangle for the element's current arranged bounds.

        @internal 
    */
    NeoWPF::Rendering::ClipRect GetClipRect() const noexcept;

    /*
        @summary
        Moves the element in layout coordinates.

        @param x
        The new X offset.

        @param y
        The new Y offset.

        @internal
    */
    void SetLayoutOffset(double x, double y) noexcept {
        if (m_offsetX != x || m_offsetY != y) {
            m_offsetX = x;
            m_offsetY = y;
            InvalidateTransformCache();
            InvalidateVisual();
        }
    }

    /*
        @summary
        Returns the X offset used for layout placement.

        @internal
    */
    double GetLayoutOffsetX() const noexcept { return m_offsetX; }

    /*
        @summary
        Returns the Y offset used for layout placement.

        @internal
    */
    double GetLayoutOffsetY() const noexcept { return m_offsetY; }

    /*
        @summary
        Animates a size change from one rectangle to another.

        @param fromWidth
        The starting width.

        @param toWidth
        The target width.

        @param fromHeight
        The starting height.

        @param toHeight
        The target height.

        @param durationSeconds
        The animation duration in seconds.
    */
    void BeginAnimation(double fromWidth, double toWidth, double fromHeight, double toHeight, float durationSeconds);

    /*
        @summary
        Animates the element toward a target alignment inside its parent.

        @param horizontal
        The target horizontal alignment.

        @param vertical
        The target vertical alignment.

        @param durationSeconds
        The animation duration in seconds.
    */
    void BeginAnimation(HorizontalAlignment horizontal, VerticalAlignment vertical, float durationSeconds);

    /*
        @summary
        Returns the width sizing mode.
    */
    LayoutSizeMode GetWidthMode() const noexcept;

    /*
        @summary
        Sets the width sizing mode.

        @param mode
        The new width mode.

        @example
        element->SetWidthMode(NeoWPF::UI::LayoutSizeMode::Stretch);
    */
    void SetWidthMode(LayoutSizeMode mode) noexcept;

    /*
        @summary
        Returns the height sizing mode.
    */
    LayoutSizeMode GetHeightMode() const noexcept;

    /*
        @summary
        Sets the height sizing mode.

        @param mode
        The new height mode.

        @example
        element->SetHeightMode(NeoWPF::UI::LayoutSizeMode::Auto);
    */
    void SetHeightMode(LayoutSizeMode mode) noexcept;

    /*
        @summary
        Sets a preferred width for the element.

        @param width
        The width to use.
    */
    double GetWidth() const noexcept;

    /*
        @summary
        Sets a preferred width for the element.

        @param width
        The width to use.
    */
    void SetWidth(double width) noexcept;

    /*
        @summary
        Returns the element's preferred height.
    */
    double GetHeight() const noexcept;

    /*
        @summary
        Sets a preferred height for the element.

        @param height
        The height to use.
    */
    void SetHeight(double height) noexcept;

    /*
        @summary
        Returns the element's layout margin.
    */
    Thickness GetMargin() const noexcept;

    /*
        @summary
        Sets the element's layout margin.
    */
    void SetMargin(Thickness margin) noexcept;

    /*
        @summary
        Sets a uniform layout margin for the element.
    */
    void SetMargin(double uniform) noexcept;

    /*
        @summary
        Sets a custom layout margin for the element.
    */
    void SetMargin(double left, double top, double right, double bottom) noexcept;

    double GetMarginLeft() const noexcept;
    double GetMarginTop() const noexcept;
    double GetMarginRight() const noexcept;
    double GetMarginBottom() const noexcept;
    double GetMarginHorizontal() const noexcept;
    double GetMarginVertical() const noexcept;

    /*
        @summary
        Returns the element's layout spacing.
    */
    Spacing GetSpacing() const noexcept;

    /*
        @summary
        Sets the element's layout spacing.
    */
    void SetSpacing(Spacing spacing) noexcept;

    /*
        @summary
        Sets a uniform layout spacing for the element.
    */
    void SetSpacing(double uniform) noexcept;

    /*
        @summary
        Returns the horizontal spacing used for layout.
    */
    double GetHorizontalSpacing() const noexcept;

    /*
        @summary
        Sets the horizontal spacing used for layout.
    */
    void SetHorizontalSpacing(double spacing) noexcept;

    /*
        @summary
        Returns the vertical spacing used for layout.
    */
    double GetVerticalSpacing() const noexcept;

    /*
        @summary
        Sets the vertical spacing used for layout.
    */
    void SetVerticalSpacing(double spacing) noexcept;

    /*
        @summary
        Returns the element's corner radius.
    */
    NeoWPF::UI::CornerRadius GetCornerRadius() const noexcept;

    /*
        @summary
        Sets the element's corner radius.

        @param cornerRadius
        The new corner radius values.
    */
    void SetCornerRadius(NeoWPF::UI::CornerRadius cornerRadius) noexcept;

    /*
        @summary
        Resolves the final width for the current sizing mode.

        @param availableWidth
        The width available to the element.

        @param desiredWidth
        The width the element wants.

        @internal
    */
    double ResolveWidth(double availableWidth, double desiredWidth) const;

    /*
        @summary
        Resolves the final height for the current sizing mode.

        @param availableHeight
        The height available to the element.

        @param desiredHeight
        The height the element wants.

        @internal
    */
    double ResolveHeight(double availableHeight, double desiredHeight) const;

    /*
        @summary
        Returns the horizontal alignment used when the element is placed in a parent.
    */
    HorizontalAlignment GetHorizontalAlignment() const noexcept;

    /*
        @summary
        Sets the horizontal alignment used when the element is placed in a parent.

        @param align
        The alignment to use.

        @example
        element->SetHorizontalAlignment(NeoWPF::UI::HorizontalAlignment::Center);
    */
    void SetHorizontalAlignment(HorizontalAlignment align) noexcept;

    /*
        @summary
        Returns the vertical alignment used when the element is placed in a parent.
    */
    VerticalAlignment GetVerticalAlignment() const noexcept;

    /*
        @summary
        Sets the vertical alignment used when the element is placed in a parent.

        @param align
        The alignment to use.

        @example
        element->SetVerticalAlignment(NeoWPF::UI::VerticalAlignment::Center);
    */
    void SetVerticalAlignment(VerticalAlignment align) noexcept;

    /*
        @summary
        Gets the current data context for the element.

        @returns
        The current data context.
    */
    std::any GetDataContext() const noexcept;

    /*
        @summary
        Sets the current data context for the element.

        @param dataContext
        The new data context value.
    */
    void SetDataContext(const std::any& dataContext) noexcept;

    template<typename T>
    void SetDataContext(const std::shared_ptr<T>& dataContext) noexcept {
        if constexpr (std::is_convertible_v<std::shared_ptr<T>, std::shared_ptr<Properties::DependencyObject>>) {
            Properties::RegisterAnyEqualityComparer<std::shared_ptr<Properties::DependencyObject>>();
            SetDataContext(std::any(std::shared_ptr<Properties::DependencyObject>(dataContext)));
        } else if constexpr (std::is_convertible_v<std::shared_ptr<T>, std::shared_ptr<UI::Data::INotifyPropertyChanged>>) {
            Properties::RegisterAnyEqualityComparer<std::shared_ptr<UI::Data::INotifyPropertyChanged>>();
            SetDataContext(std::any(std::shared_ptr<UI::Data::INotifyPropertyChanged>(dataContext)));
        } else {
            Properties::RegisterAnyEqualityComparer<std::shared_ptr<T>>();
            SetDataContext(std::any(dataContext));
        }
    }

    const Properties::DependencyObject* GetInheritanceParent() const noexcept override;
    void OnDependencyPropertyChanged(const Properties::DependencyProperty& property,
                                     const Properties::PropertyMetadata& metadata,
                                     const std::any& oldValue,
                                     const std::any& newValue,
                                     Properties::PropertyValueSource source) override;

    /*
        @summary
        Returns the element's position in absolute coordinates.

        @param outX
        Receives the absolute X position.

        @param outY
        Receives the absolute Y position.

        @internal
    */
    void GetAbsolutePosition(double& outX, double& outY) const noexcept;

    /*
        @summary
        Returns the element's layout position in absolute coordinates.

        @param outX
        Receives the absolute X position.

        @param outY
        Receives the absolute Y position.

        @internal
    */
    void GetAbsoluteLayoutPosition(double& outX, double& outY) const noexcept;

protected:
    void OnAddChild(Visual* child) override;

    void ApplyRenderTransform(double& x, double& y, double& width, double& height) const noexcept;
    NeoWPF::UI::Core::Matrix4 GetLocalTransformMatrix() const noexcept override;

public:
    static bool s_frameworkElementLogging;

private:
    double m_desiredWidth = 0.0;
    double m_desiredHeight = 0.0;
    double m_finalWidth = 0.0;
    double m_finalHeight = 0.0;
    double m_offsetX = 0.0;
    double m_offsetY = 0.0;
    LayoutSizeMode m_widthMode = LayoutSizeMode::Auto;
    double m_width = 0.0;
    double m_renderOffsetX = 0.0;
    double m_renderOffsetY = 0.0;
    LayoutSizeMode m_heightMode = LayoutSizeMode::Auto;
    double m_height = 0.0;
    Thickness m_margin;
    Spacing m_spacing;
    NeoWPF::UI::CornerRadius m_cornerRadius;
    std::any m_dataContext;
    bool m_clipToBounds = false;

protected:
    void SetRenderOffset(double x, double y) noexcept { m_renderOffsetX = x; m_renderOffsetY = y; InvalidateTransformCache(); InvalidateVisual(); }

public:
    double GetRenderOffsetX() const noexcept { return m_renderOffsetX; }
    double GetRenderOffsetY() const noexcept { return m_renderOffsetY; }

private:
    HorizontalAlignment m_horizontalAlignment = HorizontalAlignment::Stretch;
    VerticalAlignment m_verticalAlignment = VerticalAlignment::Stretch;
};

}
