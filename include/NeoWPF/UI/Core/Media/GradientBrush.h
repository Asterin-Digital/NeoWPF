/*
Apache 2.0 License — Copyright (c) 2026 Asterin Digital
See LICENSE file in the project root for full license text.
*/

/*
Do not include this in your project!
This is a NeoWPF internal header file.
Include the master umbrella instead via #include <NeoWPF/NeoWPF.h>
*/

#pragma once

#include <NeoWPF/Rendering/Paint/GradientDesc.h>
#include <NeoWPF/UI/Core/Media/Brush.h>
#include <NeoWPF/UI/Core/Collections/GradientStopCollection.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace NeoWPF::UI {

/*
    @summary
    Selects relative bounding-box coordinates or absolute gradient coordinates.
*/
enum class BrushMappingMode : std::uint8_t {
    RelativeToBoundingBox,
    Absolute,
};

/*
    @summary
    Selects the color space used to interpolate gradient stops.
*/
enum class ColorInterpolationMode : std::uint8_t {
    SRgb,
};

/*
    @summary
    Aliases the renderer's gradient spread mode.
*/
using GradientSpreadMethod = NeoWPF::Rendering::GradientSpreadMethod;

/*
    @summary
    Represents a point in gradient coordinate space.
*/
struct GradientPoint {
    /*
        @summary
        Horizontal coordinate.
    */
    double X = 0.0;

    /*
        @summary
        Vertical coordinate.
    */
    double Y = 0.0;

    /*
        @summary
        Compares two gradient points for exact equality.
    */
    constexpr bool operator==(const GradientPoint&) const noexcept = default;
};

/*
    @summary
    Base class for brushes that interpolate colors across a gradient.
*/
class GradientBrush : public Brush {
public:
    static const Properties::DependencyProperty& GradientStopsProperty();
    static const Properties::DependencyProperty& MappingModeProperty();
    static const Properties::DependencyProperty& SpreadMethodProperty();
    static const Properties::DependencyProperty& ColorInterpolationModeProperty();

    GradientBrush();
    ~GradientBrush() override;

    /*
        @summary
        Gets the gradient stops.

        @returns
        The collection used by this brush.
    */
    std::shared_ptr<GradientStopCollection> GetGradientStops() const;

    /*
        @summary
        Replaces the gradient stops.

        @param stops
        The collection to use.
    */
    void SetGradientStops(std::shared_ptr<GradientStopCollection> stops);

    /*
        @summary
        Gets the gradient coordinate mapping mode.

        @returns
        The current mapping mode.
    */
    BrushMappingMode GetMappingMode() const;

    /*
        @summary
        Sets the gradient coordinate mapping mode.

        @param mode
        The mapping mode to use.
    */
    void SetMappingMode(BrushMappingMode mode);

    /*
        @summary
        Gets how the gradient extends beyond its endpoints.

        @returns
        The current spread method.
    */
    GradientSpreadMethod GetSpreadMethod() const;

    /*
        @summary
        Sets how the gradient extends beyond its endpoints.

        @param method
        The spread method to use.
    */
    void SetSpreadMethod(GradientSpreadMethod method);

    /*
        @summary
        Gets the color interpolation mode.

        @returns
        The current interpolation mode.
    */
    ColorInterpolationMode GetColorInterpolationMode() const;

    /*
        @summary
        Sets the color interpolation mode.

        @param mode
        The interpolation mode to use.
    */
    void SetColorInterpolationMode(ColorInterpolationMode mode);

    /*
        @summary
        Resolves this brush into a renderable paint description.

        @param commandList
        The command list that stores gradient data.

        @param boundsX
        Horizontal origin of the painted bounds.

        @param boundsY
        Vertical origin of the painted bounds.

        @param boundsWidth
        Width of the painted bounds.

        @param boundsHeight
        Height of the painted bounds.

        @returns
        The resolved paint.
    */
    NeoWPF::Rendering::Paint ResolvePaint(
        NeoWPF::Rendering::CommandList& commandList,
        double boundsX,
        double boundsY,
        double boundsWidth,
        double boundsHeight) const override;

    /*
        @summary
        Resolves a representative color for fallback rendering.

        @returns
        The fallback color in ARGB format.
    */
    std::uint32_t ResolveFallbackColor() const;

protected:
    struct GradientSnapshot {
        NeoWPF::Rendering::GradientDesc::Kind kind =
            NeoWPF::Rendering::GradientDesc::Kind::Linear;
        GradientSpreadMethod spread = GradientSpreadMethod::Pad;
        std::vector<NeoWPF::Rendering::GradientStopDesc> stops;
        std::array<double, 6> geometry{};
    };

    /*
        @internal
    */
    virtual void PopulateGeometry(GradientSnapshot& snapshot) const = 0;

    /*
        @internal
    */
    void OnDependencyPropertyChanged(
        const Properties::DependencyProperty& property,
        const Properties::PropertyMetadata& metadata,
        const std::any& oldValue,
        const std::any& newValue,
        Properties::PropertyValueSource source) override;

private:
    const GradientSnapshot& GetSnapshot() const;
    void SubscribeToGradientStops();

    std::weak_ptr<GradientStopCollection> m_subscribedStops;
    std::size_t m_stopsChangedHandlerId = 0;
    mutable std::optional<GradientSnapshot> m_snapshot;
};

class LinearGradientBrush final : public GradientBrush {
public:
    static const Properties::DependencyProperty& StartPointProperty();
    static const Properties::DependencyProperty& EndPointProperty();

    /*
        @summary
        Gets the gradient start point.

        @returns
        The current start point.
    */
    GradientPoint GetStartPoint() const;

    /*
        @summary
        Sets the gradient start point.

        @param point
        The start point to use.
    */
    void SetStartPoint(GradientPoint point);

    /*
        @summary
        Gets the gradient end point.

        @returns
        The current end point.
    */
    GradientPoint GetEndPoint() const;

    /*
        @summary
        Sets the gradient end point.

        @param point
        The end point to use.
    */
    void SetEndPoint(GradientPoint point);

protected:
    /*
        @internal
    */
    void PopulateGeometry(GradientSnapshot& snapshot) const override;
};

class RadialGradientBrush final : public GradientBrush {
public:
    static const Properties::DependencyProperty& CenterProperty();
    static const Properties::DependencyProperty& GradientOriginProperty();
    static const Properties::DependencyProperty& RadiusXProperty();
    static const Properties::DependencyProperty& RadiusYProperty();

    /*
        @summary
        Gets the gradient center.

        @returns
        The current center.
    */
    GradientPoint GetCenter() const;

    /*
        @summary
        Sets the gradient center.

        @param point
        The center to use.
    */
    void SetCenter(GradientPoint point);

    /*
        @summary
        Gets the gradient origin.

        @returns
        The current origin.
    */
    GradientPoint GetGradientOrigin() const;

    /*
        @summary
        Sets the gradient origin.

        @param point
        The origin to use.
    */
    void SetGradientOrigin(GradientPoint point);

    /*
        @summary
        Gets the horizontal radius.

        @returns
        The current horizontal radius.
    */
    double GetRadiusX() const;

    /*
        @summary
        Sets the horizontal radius.

        @param radius
        The horizontal radius to use.
    */
    void SetRadiusX(double radius);

    /*
        @summary
        Gets the vertical radius.

        @returns
        The current vertical radius.
    */
    double GetRadiusY() const;

    /*
        @summary
        Sets the vertical radius.

        @param radius
        The vertical radius to use.
    */
    void SetRadiusY(double radius);

protected:
    /*
        @internal
    */
    void PopulateGeometry(GradientSnapshot& snapshot) const override;
};

}
