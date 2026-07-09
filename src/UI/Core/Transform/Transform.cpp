#include <NeoWPF/UI/Core/Transform/Transform.h>
#include <NeoWPF/UI/Animation/AnimationController.h>
#include <NeoWPF/UI/Animation/Core/Timeline.h>
#include <NeoWPF/UI/Animation/Storyboards/Storyboard.h>
#include <cmath>

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
    float cos_a = std::cos(angleRadians);
    float sin_a = std::sin(angleRadians);
    m.m[0][0] = cos_a;
    m.m[0][1] = sin_a;
    m.m[1][0] = -sin_a;
    m.m[1][1] = cos_a;
    return m;
}

ScaleTransform::ScaleTransform(float scaleX, float scaleY, float centerX, float centerY)
    : m_scaleX(scaleX), m_scaleY(scaleY), m_centerX(centerX), m_centerY(centerY) {}

float ScaleTransform::GetScaleX() const { return m_scaleX; }
float ScaleTransform::GetScaleY() const { return m_scaleY; }
float ScaleTransform::GetCenterX() const { return m_centerX; }
float ScaleTransform::GetCenterY() const { return m_centerY; }

void ScaleTransform::SetScaleX(float value) { m_scaleX = value; }
void ScaleTransform::SetScaleY(float value) { m_scaleY = value; }
void ScaleTransform::SetCenterX(float value) { m_centerX = value; }
void ScaleTransform::SetCenterY(float value) { m_centerY = value; }

Matrix4 ScaleTransform::GetMatrix() const {
    Matrix4 toCenter = Matrix4::Translation(-m_centerX, -m_centerY);
    Matrix4 scale = Matrix4::Scale(m_scaleX, m_scaleY);
    Matrix4 fromCenter = Matrix4::Translation(m_centerX, m_centerY);
    return fromCenter * scale * toCenter;
}

std::shared_ptr<Transform> ScaleTransform::Clone() const {
    return std::make_shared<ScaleTransform>(m_scaleX, m_scaleY, m_centerX, m_centerY);
}

void ScaleTransform::AnimateTo(float scaleValue, float durationSeconds) {
    using namespace NeoWPF::UI::Animation;

    auto storyboard = std::make_shared<Storyboard>();
    auto scaleXTimeline = std::make_shared<Timeline>(m_scaleX, scaleValue, durationSeconds, [this](float value) {
        SetScaleX(value);
        if (m_invalidateCallback) {
            m_invalidateCallback();
        }
    });
    auto scaleYTimeline = std::make_shared<Timeline>(m_scaleY, scaleValue, durationSeconds, [this](float value) {
        SetScaleY(value);
        if (m_invalidateCallback) {
            m_invalidateCallback();
        }
    });

    storyboard->AddTimeline(scaleXTimeline);
    storyboard->AddTimeline(scaleYTimeline);
    NeoWPF::UI::AddStoryboard(storyboard);
}

TranslateTransform::TranslateTransform(float x, float y)
    : m_x(x), m_y(y) {}

float TranslateTransform::GetX() const { return m_x; }
float TranslateTransform::GetY() const { return m_y; }

void TranslateTransform::SetX(float value) { m_x = value; }
void TranslateTransform::SetY(float value) { m_y = value; }

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

void RotateTransform::SetAngle(float value) { m_angle = value; }
void RotateTransform::SetCenterX(float value) { m_centerX = value; }
void RotateTransform::SetCenterY(float value) { m_centerY = value; }

Matrix4 RotateTransform::GetMatrix() const {
    Matrix4 toCenter = Matrix4::Translation(-m_centerX, -m_centerY);
    Matrix4 rotation = Matrix4::Rotation(m_angle);
    Matrix4 fromCenter = Matrix4::Translation(m_centerX, m_centerY);
    return fromCenter * rotation * toCenter;
}

std::shared_ptr<Transform> RotateTransform::Clone() const {
    return std::make_shared<RotateTransform>(m_angle, m_centerX, m_centerY);
}

MatrixTransform::MatrixTransform(const Matrix4& matrix)
    : m_matrix(matrix) {}

Matrix4 MatrixTransform::GetMatrixValue() const { return m_matrix; }
void MatrixTransform::SetMatrix(const Matrix4& matrix) { m_matrix = matrix; }

Matrix4 MatrixTransform::GetMatrix() const {
    return m_matrix;
}

std::shared_ptr<Transform> MatrixTransform::Clone() const {
    return std::make_shared<MatrixTransform>(m_matrix);
}

TransformGroup::TransformGroup() {}

void TransformGroup::AddChild(std::shared_ptr<Transform> transform) {
    if (transform) {
        m_children.push_back(transform);
    }
}

void TransformGroup::ClearChildren() {
    m_children.clear();
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
