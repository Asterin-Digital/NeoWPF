#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <cmath>
#include <utility>

namespace NeoWPF::UI {

namespace Core {

Matrix4::Matrix4() {
    m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
    m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;
    m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = 0;
    m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                result.m[i][j] += m[i][k] * other.m[k][j];
            }
        }
    }
    return result;
}

Matrix4 Matrix4::Identity() {
    return Matrix4();
}

Matrix4 Matrix4::Translation(float tx, float ty) {
    Matrix4 m;
    m.m[3][0] = tx;
    m.m[3][1] = ty;
    return m;
}

Matrix4 Matrix4::Scale(float sx, float sy) {
    Matrix4 m;
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    return m;
}

Matrix4 Matrix4::Rotation(float angleRadians) {
    Matrix4 m;
    const float cos_a = std::cos(angleRadians);
    const float sin_a = std::sin(angleRadians);
    m.m[0][0] = cos_a;
    m.m[0][1] = -sin_a;
    m.m[1][0] = sin_a;
    m.m[1][1] = cos_a;
    return m;
}

std::optional<float> Transform::TryGetRotation() const noexcept {
    return std::nullopt;
}

Matrix4 Transform::GetPositionMatrix() const {
    return GetMatrix();
}

bool Transform::HasRotation() const noexcept {
    return false;
}

std::size_t Transform::AddChangedHandler(std::function<void()> handler)
{
    return m_changed.Subscribe(std::move(handler));
}

void Transform::RemoveChangedHandler(std::size_t handlerId)
{
    m_changed.Unsubscribe(handlerId);
}

void Transform::NotifyChanged()
{
    m_changed.Publish();
}

TransformSubscription::TransformSubscription(TransformSubscription&& other) noexcept
    : m_transform(std::move(other.m_transform)),
      m_handlerId(std::exchange(other.m_handlerId, 0))
{
}

TransformSubscription& TransformSubscription::operator=(TransformSubscription&& other) noexcept
{
    if (this != &other) {
        Reset();
        m_transform = std::move(other.m_transform);
        m_handlerId = std::exchange(other.m_handlerId, 0);
    }
    return *this;
}

TransformSubscription::~TransformSubscription()
{
    Reset();
}

void TransformSubscription::Subscribe(
    const std::shared_ptr<Transform>& transform,
    std::function<void()> handler)
{
    Reset();
    if (!transform || !handler) {
        return;
    }

    m_handlerId = transform->AddChangedHandler(std::move(handler));
    if (m_handlerId != 0) {
        m_transform = transform;
    }
}

void TransformSubscription::Reset() noexcept
{
    if (const auto transform = m_transform.lock(); transform && m_handlerId != 0) {
        transform->RemoveChangedHandler(m_handlerId);
    }
    m_transform.reset();
    m_handlerId = 0;
}

void Matrix4::TransformPoint(double x, double y, double& outX, double& outY) const noexcept {
    outX = x * m[0][0] + y * m[1][0] + m[3][0];
    outY = x * m[0][1] + y * m[1][1] + m[3][1];
}

std::optional<Matrix4> Matrix4::TryInverse2D() const noexcept
{
    if (m[0][2] != 0.0f || m[1][2] != 0.0f || m[2][0] != 0.0f ||
        m[2][1] != 0.0f || m[2][2] != 1.0f || m[3][2] != 0.0f ||
        m[0][3] != 0.0f || m[1][3] != 0.0f || m[2][3] != 0.0f ||
        m[3][3] != 1.0f) {
        return std::nullopt;
    }

    const float determinant = m[0][0] * m[1][1] - m[0][1] * m[1][0];
    if (!std::isfinite(m[0][0]) || !std::isfinite(m[0][1]) ||
        !std::isfinite(m[1][0]) || !std::isfinite(m[1][1]) ||
        !std::isfinite(m[3][0]) || !std::isfinite(m[3][1]) ||
        !std::isfinite(determinant) || determinant == 0.0f) {
        return std::nullopt;
    }

    const float inverseDeterminant = 1.0f / determinant;
    Matrix4 inverse;
    inverse.m[0][0] = m[1][1] * inverseDeterminant;
    inverse.m[0][1] = -m[0][1] * inverseDeterminant;
    inverse.m[1][0] = -m[1][0] * inverseDeterminant;
    inverse.m[1][1] = m[0][0] * inverseDeterminant;
    inverse.m[3][0] = -(m[3][0] * inverse.m[0][0] + m[3][1] * inverse.m[1][0]);
    inverse.m[3][1] = -(m[3][0] * inverse.m[0][1] + m[3][1] * inverse.m[1][1]);
    return inverse;
}

ScaleTransform::ScaleTransform(float scaleX, float scaleY, float centerX, float centerY)
    : m_scaleX(scaleX), m_scaleY(scaleY), m_centerX(centerX), m_centerY(centerY) {}

float ScaleTransform::GetScaleX() const { return m_scaleX; }
float ScaleTransform::GetScaleY() const { return m_scaleY; }
float ScaleTransform::GetCenterX() const { return m_centerX; }
float ScaleTransform::GetCenterY() const { return m_centerY; }

void ScaleTransform::SetScaleX(float value) { m_scaleX = value; NotifyChanged(); }
void ScaleTransform::SetScaleY(float value) { m_scaleY = value; NotifyChanged(); }
void ScaleTransform::SetCenterX(float value) { m_centerX = value; NotifyChanged(); }
void ScaleTransform::SetCenterY(float value) { m_centerY = value; NotifyChanged(); }

Matrix4 ScaleTransform::GetMatrix() const {
    Matrix4 toCenter = Matrix4::Translation(-m_centerX, -m_centerY);
    Matrix4 scale = Matrix4::Scale(m_scaleX, m_scaleY);
    Matrix4 fromCenter = Matrix4::Translation(m_centerX, m_centerY);
    return toCenter * scale * fromCenter;
}

std::shared_ptr<Transform> ScaleTransform::Clone() const {
    return std::make_shared<ScaleTransform>(m_scaleX, m_scaleY, m_centerX, m_centerY);
}

void ScaleTransform::AnimateTo(float scaleValue, float durationSeconds) {
    if (m_animationHandle != 0) {
        NeoWPF::UI::StopAnimation(m_animationHandle);
    }

    const float fromScale = m_scaleX;
    m_animationHandle = NeoWPF::UI::BeginValueAnimation(
        fromScale,
        scaleValue,
        durationSeconds,
        [this](float value) {
            SetScaleX(value);
            SetScaleY(value);
        });
}

TranslateTransform::TranslateTransform(float x, float y)
    : m_x(x), m_y(y) {}

float TranslateTransform::GetX() const { return m_x; }
float TranslateTransform::GetY() const { return m_y; }

void TranslateTransform::SetX(float value) { m_x = value; NotifyChanged(); }
void TranslateTransform::SetY(float value) { m_y = value; NotifyChanged(); }

Matrix4 TranslateTransform::GetMatrix() const {
    return Matrix4::Translation(m_x, m_y);
}

std::shared_ptr<Transform> TranslateTransform::Clone() const {
    return std::make_shared<TranslateTransform>(m_x, m_y);
}

RotateTransform::RotateTransform(float angle, float centerX, float centerY)
    : m_angle(angle), m_centerX(centerX), m_centerY(centerY) {}

float RotateTransform::GetAngle() const { return m_angle; }
float RotateTransform::GetCenterX() const { return m_centerX; }
float RotateTransform::GetCenterY() const { return m_centerY; }

void RotateTransform::SetAngle(float value) { m_angle = value; NotifyChanged(); }
void RotateTransform::SetCenterX(float value) { m_centerX = value; NotifyChanged(); }
void RotateTransform::SetCenterY(float value) { m_centerY = value; NotifyChanged(); }

Matrix4 RotateTransform::GetMatrix() const {
    Matrix4 toCenter = Matrix4::Translation(-m_centerX, -m_centerY);
    Matrix4 rotation = Matrix4::Rotation(m_angle);
    Matrix4 fromCenter = Matrix4::Translation(m_centerX, m_centerY);
    return toCenter * rotation * fromCenter;
}

std::optional<float> RotateTransform::TryGetRotation() const noexcept {
    return m_angle;
}

bool RotateTransform::HasRotation() const noexcept {
    return true;
}

Matrix4 RotateTransform::GetPositionMatrix() const {
    return Matrix4::Identity();
}

std::shared_ptr<Transform> RotateTransform::Clone() const {
    return std::make_shared<RotateTransform>(m_angle, m_centerX, m_centerY);
}

MatrixTransform::MatrixTransform(const Matrix4& matrix)
    : m_matrix(matrix) {}

Matrix4 MatrixTransform::GetMatrixValue() const { return m_matrix; }
void MatrixTransform::SetMatrix(const Matrix4& matrix) { m_matrix = matrix; NotifyChanged(); }

Matrix4 MatrixTransform::GetMatrix() const {
    return m_matrix;
}

std::shared_ptr<Transform> MatrixTransform::Clone() const {
    return std::make_shared<MatrixTransform>(m_matrix);
}

TransformGroup::TransformGroup() {}

TransformGroup::~TransformGroup() = default;

void TransformGroup::AddChild(std::shared_ptr<Transform> transform) {
    if (transform) {
        TransformSubscription subscription;
        subscription.Subscribe(transform, [this] {
            NotifyChanged();
        });
        m_childChangedSubscriptions.push_back(std::move(subscription));
        m_children.push_back(std::move(transform));
        NotifyChanged();
    }
}

void TransformGroup::ClearChildren() {
    if (m_children.empty()) {
        return;
    }
    m_childChangedSubscriptions.clear();
    m_children.clear();
    NotifyChanged();
}

size_t TransformGroup::GetChildCount() const {
    return m_children.size();
}

std::shared_ptr<Transform> TransformGroup::GetChild(size_t index) const {
    if (index < m_children.size()) {
        return m_children[index];
    }
    return nullptr;
}

Matrix4 TransformGroup::GetMatrix() const {
    Matrix4 result = Matrix4::Identity();
    for (const auto& child : m_children) {
        if (child) {
            result = result * child->GetMatrix();
        }
    }
    return result;
}

std::optional<float> TransformGroup::TryGetRotation() const noexcept {
    float rotation = 0.0f;
    for (const auto& child : m_children) {
        if (!child) {
            continue;
        }

        const auto childRotation = child->TryGetRotation();
        if (!childRotation) {
            return std::nullopt;
        }
        rotation += *childRotation;
    }
    return rotation;
}

bool TransformGroup::HasRotation() const noexcept {
    for (const auto& child : m_children) {
        if (child && child->HasRotation()) {
            return true;
        }
    }
    return false;
}

Matrix4 TransformGroup::GetPositionMatrix() const {
    Matrix4 result = Matrix4::Identity();
    for (const auto& child : m_children) {
        if (child) {
            result = result * child->GetPositionMatrix();
        }
    }
    return result;
}

std::shared_ptr<Transform> TransformGroup::Clone() const {
    auto cloned = std::make_shared<TransformGroup>();
    for (const auto& child : m_children) {
        if (child) {
            cloned->AddChild(child->Clone());
        }
    }
    return cloned;
}

}

}
