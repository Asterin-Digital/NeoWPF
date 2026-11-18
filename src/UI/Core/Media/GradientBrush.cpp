#include <NeoWPF/UI/Core/Media/GradientBrush.h>
#include <NeoWPF/Rendering/Renderer.h>

#include <algorithm>
#include <cmath>
#include <iterator>

namespace NeoWPF::UI {
namespace {

struct Affine2D {
    double m00 = 1.0;
    double m01 = 0.0;
    double m10 = 0.0;
    double m11 = 1.0;
    double tx = 0.0;
    double ty = 0.0;

    static Affine2D FromMatrix(const Core::Matrix4& matrix)
    {
        return {
            matrix.m[0][0], matrix.m[0][1],
            matrix.m[1][0], matrix.m[1][1],
            matrix.m[3][0], matrix.m[3][1],
        };
    }

    std::optional<Affine2D> TryInverse() const
    {
        const double determinant = m00 * m11 - m01 * m10;
        if (!std::isfinite(m00) || !std::isfinite(m01) ||
            !std::isfinite(m10) || !std::isfinite(m11) ||
            !std::isfinite(tx) || !std::isfinite(ty) ||
            !std::isfinite(determinant) || determinant == 0.0) {
            return std::nullopt;
        }

        const double inverseDeterminant = 1.0 / determinant;
        Affine2D inverse;
        inverse.m00 = m11 * inverseDeterminant;
        inverse.m01 = -m01 * inverseDeterminant;
        inverse.m10 = -m10 * inverseDeterminant;
        inverse.m11 = m00 * inverseDeterminant;
        inverse.tx = -(tx * inverse.m00 + ty * inverse.m10);
        inverse.ty = -(tx * inverse.m01 + ty * inverse.m11);
        return inverse;
    }

    Affine2D operator*(const Affine2D& other) const
    {
        return {
            m00 * other.m00 + m01 * other.m10,
            m00 * other.m01 + m01 * other.m11,
            m10 * other.m00 + m11 * other.m10,
            m10 * other.m01 + m11 * other.m11,
            tx * other.m00 + ty * other.m10 + other.tx,
            tx * other.m01 + ty * other.m11 + other.ty,
        };
    }
};

Affine2D Translation(double x, double y)
{
    Affine2D result;
    result.tx = x;
    result.ty = y;
    return result;
}

Affine2D Scale(double x, double y)
{
    Affine2D result;
    result.m00 = x;
    result.m11 = y;
    return result;
}

bool Is2DAffine(const Core::Matrix4& matrix)
{
    return matrix.m[0][2] == 0.0f && matrix.m[1][2] == 0.0f &&
        matrix.m[2][0] == 0.0f && matrix.m[2][1] == 0.0f &&
        matrix.m[2][2] == 1.0f && matrix.m[3][2] == 0.0f &&
        matrix.m[0][3] == 0.0f && matrix.m[1][3] == 0.0f &&
        matrix.m[2][3] == 0.0f && matrix.m[3][3] == 1.0f;
}

bool TryCreateWorldToBrushMapping(
    BrushMappingMode mode,
    double boundsX,
    double boundsY,
    double boundsWidth,
    double boundsHeight,
    const std::shared_ptr<Core::Transform>& relativeTransform,
    const std::shared_ptr<Core::Transform>& transform,
    Affine2D& mapping)
{
    if (mode == BrushMappingMode::RelativeToBoundingBox &&
        (boundsWidth == 0.0 || boundsHeight == 0.0)) {
        return false;
    }
    if (mode == BrushMappingMode::Absolute && relativeTransform &&
        (boundsWidth == 0.0 || boundsHeight == 0.0)) {
        return false;
    }

    const auto getInverse = [](const std::shared_ptr<Core::Transform>& value) {
        if (!value) {
            return std::optional<Affine2D>(Affine2D{});
        }
        const auto matrix = value->GetMatrix();
        if (!Is2DAffine(matrix)) {
            return std::optional<Affine2D>{};
        }
        return Affine2D::FromMatrix(matrix).TryInverse();
    };
    const auto inverseRelative = getInverse(relativeTransform);
    const auto inverseTransform = getInverse(transform);
    if (!inverseRelative || !inverseTransform) {
        return false;
    }

    if (mode == BrushMappingMode::RelativeToBoundingBox) {
        const auto inverseBounds = Translation(-boundsX, -boundsY)
            * Scale(1.0 / boundsWidth, 1.0 / boundsHeight);
        mapping = *inverseTransform * inverseBounds * *inverseRelative;
    } else {
        const auto relativeTransformToAbsolute = relativeTransform
            ? Scale(1.0 / boundsWidth, 1.0 / boundsHeight)
                * *inverseRelative * Scale(boundsWidth, boundsHeight)
            : Affine2D{};
        mapping = *inverseTransform * Translation(-boundsX, -boundsY)
            * relativeTransformToAbsolute;
    }
    return true;
}

std::uint32_t ApplyOpacity(std::uint32_t color, double opacity)
{
    const auto clampedOpacity = (std::clamp)(opacity, 0.0, 1.0);
    const auto alpha = static_cast<std::uint32_t>(
        static_cast<double>((color >> 24) & 0xFFu) * clampedOpacity);
    return (color & 0x00FFFFFFu) | (alpha << 24);
}

}

GradientBrush::GradientBrush()
{
    SetGradientStops(std::make_shared<GradientStopCollection>());
}

GradientBrush::~GradientBrush()
{
    if (const auto stops = m_subscribedStops.lock();
        stops && m_stopsChangedHandlerId != 0) {
        stops->RemoveChangedHandler(m_stopsChangedHandlerId);
    }
}

std::shared_ptr<GradientStopCollection> GradientBrush::GetGradientStops() const
{
    return GetValue<std::shared_ptr<GradientStopCollection>>(GradientStopsProperty());
}

void GradientBrush::SetGradientStops(std::shared_ptr<GradientStopCollection> stops)
{
    SetValue(GradientStopsProperty(), std::move(stops));
}

BrushMappingMode GradientBrush::GetMappingMode() const
{
    return GetValue<BrushMappingMode>(MappingModeProperty());
}

void GradientBrush::SetMappingMode(BrushMappingMode mode)
{
    SetValue(MappingModeProperty(), mode);
}

GradientSpreadMethod GradientBrush::GetSpreadMethod() const
{
    return GetValue<GradientSpreadMethod>(SpreadMethodProperty());
}

void GradientBrush::SetSpreadMethod(GradientSpreadMethod method)
{
    SetValue(SpreadMethodProperty(), method);
}

ColorInterpolationMode GradientBrush::GetColorInterpolationMode() const
{
    return GetValue<ColorInterpolationMode>(ColorInterpolationModeProperty());
}

void GradientBrush::SetColorInterpolationMode(ColorInterpolationMode mode)
{
    SetValue(ColorInterpolationModeProperty(), mode);
}

NeoWPF::Rendering::Paint GradientBrush::ResolvePaint(
    NeoWPF::Rendering::CommandList& commandList,
    double boundsX,
    double boundsY,
    double boundsWidth,
    double boundsHeight) const
{
    const auto& snapshot = GetSnapshot();
    if (snapshot.stops.empty()) {
        return NeoWPF::Rendering::Paint::Solid(0);
    }

    const double opacity = (std::clamp)(GetOpacity(), 0.0, 1.0);
    if (snapshot.stops.size() == 1) {
        return NeoWPF::Rendering::Paint::Solid(
            ApplyOpacity(snapshot.stops.front().color, opacity));
    }

    const auto lastStopPaint = [&] {
        return NeoWPF::Rendering::Paint::Solid(
            ApplyOpacity(snapshot.stops.back().color, opacity));
    };

    if (GetMappingMode() == BrushMappingMode::RelativeToBoundingBox &&
        (boundsWidth == 0.0 || boundsHeight == 0.0)) {
        return lastStopPaint();
    }

    Affine2D worldToBrush;
    if (!TryCreateWorldToBrushMapping(
            GetMappingMode(), boundsX, boundsY, boundsWidth, boundsHeight,
            GetRelativeTransform(), GetTransform(), worldToBrush)) {
        return NeoWPF::Rendering::Paint::Solid(0);
    }

    NeoWPF::Rendering::GradientDesc description;
    description.kind = snapshot.kind;
    description.spread = snapshot.spread;
    description.opacity = static_cast<float>(opacity);
    description.stopOffset = static_cast<std::uint32_t>(commandList.gradientStops.size());
    description.stopCount = static_cast<std::uint32_t>(snapshot.stops.size());

    const auto& geometry = snapshot.geometry;
    if (snapshot.kind == NeoWPF::Rendering::GradientDesc::Kind::Linear) {
        const double startX = geometry[0];
        const double startY = geometry[1];
        const double deltaX = geometry[2] - startX;
        const double deltaY = geometry[3] - startY;
        const double lengthSquared = deltaX * deltaX + deltaY * deltaY;
        if (!std::isfinite(lengthSquared) || lengthSquared == 0.0) {
            return lastStopPaint();
        }
        description.mapping = {
            (worldToBrush.m00 * deltaX + worldToBrush.m01 * deltaY) / lengthSquared,
            0.0,
            (worldToBrush.m10 * deltaX + worldToBrush.m11 * deltaY) / lengthSquared,
            0.0,
            ((worldToBrush.tx - startX) * deltaX +
             (worldToBrush.ty - startY) * deltaY) / lengthSquared,
            0.0,
        };
    } else {
        const double centerX = geometry[0];
        const double centerY = geometry[1];
        const double radiusX = geometry[4];
        const double radiusY = geometry[5];
        if (!std::isfinite(radiusX) || !std::isfinite(radiusY) ||
            radiusX <= 0.0 || radiusY <= 0.0) {
            return lastStopPaint();
        }

        description.mapping = {
            worldToBrush.m00 / radiusX,
            worldToBrush.m01 / radiusY,
            worldToBrush.m10 / radiusX,
            worldToBrush.m11 / radiusY,
            (worldToBrush.tx - centerX) / radiusX,
            (worldToBrush.ty - centerY) / radiusY,
        };
        description.focalX = (geometry[2] - centerX) / radiusX;
        description.focalY = (geometry[3] - centerY) / radiusY;
    }

    if (!std::all_of(
            description.mapping.begin(), description.mapping.end(),
            [](double value) { return std::isfinite(value); }) ||
        !std::isfinite(description.focalX) || !std::isfinite(description.focalY)) {
        return NeoWPF::Rendering::Paint::Solid(0);
    }

    commandList.gradientStops.insert(
        commandList.gradientStops.end(), snapshot.stops.begin(), snapshot.stops.end());
    const auto gradientIndex = static_cast<std::uint32_t>(commandList.gradients.size());
    commandList.gradients.push_back(description);
    return snapshot.kind == NeoWPF::Rendering::GradientDesc::Kind::Linear
        ? NeoWPF::Rendering::Paint::LinearGradient(gradientIndex)
        : NeoWPF::Rendering::Paint::RadialGradient(gradientIndex);
}

std::uint32_t GradientBrush::ResolveFallbackColor() const
{
    const auto& stops = GetSnapshot().stops;
    if (stops.empty()) {
        return 0;
    }

    constexpr double midpoint = 0.5;
    const auto right = std::upper_bound(
        stops.begin(), stops.end(), midpoint,
        [](double offset, const auto& stop) { return offset < stop.offset; });
    if (right == stops.begin()) {
        return ApplyOpacity(stops.front().color, GetOpacity());
    }
    const auto left = std::prev(right);
    if (right == stops.end()) {
        return ApplyOpacity(left->color, GetOpacity());
    }

    const double range = static_cast<double>(right->offset - left->offset);
    const double amount = range == 0.0 ? 1.0 : (midpoint - left->offset) / range;
    std::uint32_t color = 0;
    for (const auto shift : {24, 16, 8, 0}) {
        const double leftChannel = static_cast<double>((left->color >> shift) & 0xFFu);
        const double rightChannel = static_cast<double>((right->color >> shift) & 0xFFu);
        const auto channel = static_cast<std::uint32_t>(
            std::lround(leftChannel + (rightChannel - leftChannel) * amount));
        color |= channel << shift;
    }
    return ApplyOpacity(color, GetOpacity());
}

void GradientBrush::OnDependencyPropertyChanged(
    const Properties::DependencyProperty& property,
    const Properties::PropertyMetadata& metadata,
    const std::any& oldValue,
    const std::any& newValue,
    Properties::PropertyValueSource source)
{
    m_snapshot.reset();
    if (property.GetName() == "GradientStops") {
        SubscribeToGradientStops();
    }
    Brush::OnDependencyPropertyChanged(property, metadata, oldValue, newValue, source);
}

const GradientBrush::GradientSnapshot& GradientBrush::GetSnapshot() const
{
    if (!m_snapshot) {
        GradientSnapshot snapshot;
        snapshot.spread = GetSpreadMethod();
        if (const auto stops = GetGradientStops()) {
            snapshot.stops.reserve(stops->size());
            for (const auto& stop : *stops) {
                if (!stop) {
                    continue;
                }
                const double offset = stop->GetOffset();
                snapshot.stops.push_back({
                    static_cast<float>(std::isfinite(offset)
                        ? (std::clamp)(offset, 0.0, 1.0)
                        : 0.0),
                    stop->GetColor().ToArgb(),
                });
            }
        }
        std::stable_sort(snapshot.stops.begin(), snapshot.stops.end(),
            [](const auto& left, const auto& right) {
                return left.offset < right.offset;
            });
        PopulateGeometry(snapshot);
        m_snapshot.emplace(std::move(snapshot));
    }
    return *m_snapshot;
}

void GradientBrush::SubscribeToGradientStops()
{
    if (const auto stops = m_subscribedStops.lock();
        stops && m_stopsChangedHandlerId != 0) {
        stops->RemoveChangedHandler(m_stopsChangedHandlerId);
    }
    m_subscribedStops.reset();
    m_stopsChangedHandlerId = 0;

    if (const auto stops = GetGradientStops()) {
        m_subscribedStops = stops;
        m_stopsChangedHandlerId = stops->AddChangedHandler([this] {
            m_snapshot.reset();
            NotifyChanged();
        });
    }
}

GradientPoint LinearGradientBrush::GetStartPoint() const
{
    return GetValue<GradientPoint>(StartPointProperty());
}

void LinearGradientBrush::SetStartPoint(GradientPoint point)
{
    SetValue(StartPointProperty(), point);
}

GradientPoint LinearGradientBrush::GetEndPoint() const
{
    return GetValue<GradientPoint>(EndPointProperty());
}

void LinearGradientBrush::SetEndPoint(GradientPoint point)
{
    SetValue(EndPointProperty(), point);
}

void LinearGradientBrush::PopulateGeometry(GradientSnapshot& snapshot) const
{
    const auto start = GetStartPoint();
    const auto end = GetEndPoint();
    snapshot.kind = NeoWPF::Rendering::GradientDesc::Kind::Linear;
    snapshot.geometry = {start.X, start.Y, end.X, end.Y, 0.0, 0.0};
}

GradientPoint RadialGradientBrush::GetCenter() const
{
    return GetValue<GradientPoint>(CenterProperty());
}

void RadialGradientBrush::SetCenter(GradientPoint point)
{
    SetValue(CenterProperty(), point);
}

GradientPoint RadialGradientBrush::GetGradientOrigin() const
{
    return GetValue<GradientPoint>(GradientOriginProperty());
}

void RadialGradientBrush::SetGradientOrigin(GradientPoint point)
{
    SetValue(GradientOriginProperty(), point);
}

double RadialGradientBrush::GetRadiusX() const
{
    return GetValue<double>(RadiusXProperty());
}

void RadialGradientBrush::SetRadiusX(double radius)
{
    SetValue(RadiusXProperty(), radius);
}

double RadialGradientBrush::GetRadiusY() const
{
    return GetValue<double>(RadiusYProperty());
}

void RadialGradientBrush::SetRadiusY(double radius)
{
    SetValue(RadiusYProperty(), radius);
}

void RadialGradientBrush::PopulateGeometry(GradientSnapshot& snapshot) const
{
    const auto center = GetCenter();
    const auto origin = GetGradientOrigin();
    const double radiusX = GetRadiusX();
    const double radiusY = GetRadiusY();
    double clampedOriginX = origin.X;
    double clampedOriginY = origin.Y;
    const double originX = (origin.X - center.X) / (radiusX == 0.0 ? 1.0 : radiusX);
    const double originY = (origin.Y - center.Y) / (radiusY == 0.0 ? 1.0 : radiusY);
    const double originLength = std::hypot(originX, originY);
    if (originLength >= 1.0 && std::isfinite(originLength)) {
        const double scale = (1.0 - 1e-6) / originLength;
        clampedOriginX = center.X + (origin.X - center.X) * scale;
        clampedOriginY = center.Y + (origin.Y - center.Y) * scale;
    }

    snapshot.kind = NeoWPF::Rendering::GradientDesc::Kind::Radial;
    snapshot.geometry = {
        center.X, center.Y, clampedOriginX, clampedOriginY, radiusX, radiusY,
    };
}

}
