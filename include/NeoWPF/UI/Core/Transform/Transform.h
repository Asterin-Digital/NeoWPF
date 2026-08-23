#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <cstdint>

namespace NeoWPF::UI {
class UIElement;

namespace Core {

struct Matrix4 {
    float m[4][4];

    Matrix4();
    Matrix4 operator*(const Matrix4& other) const;

    static Matrix4 Identity();
    static Matrix4 Translation(float tx, float ty);
    static Matrix4 Scale(float sx, float sy);
    static Matrix4 Rotation(float angleRadians);
};

class Transform {
public:
    virtual ~Transform() = default;
    virtual Matrix4 GetMatrix() const = 0;
    virtual std::shared_ptr<Transform> Clone() const = 0;
};

struct FloatAnimation {
    FloatAnimation(float fromValue = 0.0f, float toValue = 0.0f, float durationSeconds = 0.0f)
        : from(fromValue), to(toValue), durationSeconds(durationSeconds) {}

    float from = 0.0f;
    float to = 0.0f;
    float durationSeconds = 0.0f;
};

class ScaleTransform : public Transform {
public:
    ScaleTransform(float scaleX = 1.0f, float scaleY = 1.0f, float centerX = 0.0f, float centerY = 0.0f);

    float GetScaleX() const;
    float GetScaleY() const;
    float GetCenterX() const;
    float GetCenterY() const;

    void SetScaleX(float value);
    void SetScaleY(float value);
    void SetCenterX(float value);
    void SetCenterY(float value);
    void SetInvalidateCallback(std::function<void()> callback);

    Matrix4 GetMatrix() const override;
    std::shared_ptr<Transform> Clone() const override;
    void AnimateTo(float scaleValue, float durationSeconds);

private:
    float m_scaleX;
    float m_scaleY;
    float m_centerX;
    float m_centerY;
    std::function<void()> m_invalidateCallback;
    std::uint64_t m_animationHandle = 0;
    friend class NeoWPF::UI::UIElement;
};

class TranslateTransform : public Transform {
public:
    TranslateTransform(float x = 0.0f, float y = 0.0f);

    float GetX() const;
    float GetY() const;

    void SetX(float value);
    void SetY(float value);

    Matrix4 GetMatrix() const override;
    std::shared_ptr<Transform> Clone() const override;

private:
    float m_x;
    float m_y;
};

class RotateTransform : public Transform {
public:
    RotateTransform(float angle = 0.0f, float centerX = 0.0f, float centerY = 0.0f);

    float GetAngle() const;
    float GetCenterX() const;
    float GetCenterY() const;

    void SetAngle(float value);
    void SetCenterX(float value);
    void SetCenterY(float value);
    void SetInvalidateCallback(std::function<void()> callback);

    Matrix4 GetMatrix() const override;
    std::shared_ptr<Transform> Clone() const override;

private:
    float m_angle;
    float m_centerX;
    float m_centerY;
    std::function<void()> m_invalidateCallback;
    friend class NeoWPF::UI::UIElement;
};

class MatrixTransform : public Transform {
public:
    MatrixTransform(const Matrix4& matrix = Matrix4::Identity());

    Matrix4 GetMatrixValue() const;
    void SetMatrix(const Matrix4& matrix);

    Matrix4 GetMatrix() const override;
    std::shared_ptr<Transform> Clone() const override;

private:
    Matrix4 m_matrix;
};

class TransformGroup : public Transform {
public:
    TransformGroup();

    void AddChild(std::shared_ptr<Transform> transform);
    void ClearChildren();
    size_t GetChildCount() const;
    std::shared_ptr<Transform> GetChild(size_t index) const;

    Matrix4 GetMatrix() const override;
    std::shared_ptr<Transform> Clone() const override;

private:
    std::vector<std::shared_ptr<Transform>> m_children;
};

} // namespace Core

using Core::Matrix4;
using Core::Transform;
using Core::FloatAnimation;
using Core::ScaleTransform;
using Core::TranslateTransform;
using Core::RotateTransform;
using Core::MatrixTransform;
using Core::TransformGroup;

} // namespace NeoWPF::UI
