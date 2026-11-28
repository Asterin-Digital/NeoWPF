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

#include <memory>
#include <vector>
#include <functional>
#include <cstdint>
#include <optional>
#include <cstddef>
#include <NeoWPF/Core/Events/Notification.h>

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

    void TransformPoint(double x, double y, double& outX, double& outY) const noexcept;
    std::optional<Matrix4> TryInverse2D() const noexcept;
};

class Transform {
public:
    virtual ~Transform() = default;
    virtual Matrix4 GetMatrix() const = 0;






    virtual Matrix4 GetPositionMatrix() const;






    virtual bool HasRotation() const noexcept;
    virtual std::optional<float> TryGetRotation() const noexcept;
    virtual std::shared_ptr<Transform> Clone() const = 0;

    std::size_t AddChangedHandler(std::function<void()> handler);
    void RemoveChangedHandler(std::size_t handlerId);

protected:
    void NotifyChanged();

private:
    NeoWPF::Core::Notification<> m_changed;
};

class TransformSubscription final {
public:
    TransformSubscription() = default;
    TransformSubscription(const TransformSubscription&) = delete;
    TransformSubscription& operator=(const TransformSubscription&) = delete;
    TransformSubscription(TransformSubscription&& other) noexcept;
    TransformSubscription& operator=(TransformSubscription&& other) noexcept;
    ~TransformSubscription();

    void Subscribe(const std::shared_ptr<Transform>& transform,
                   std::function<void()> handler);
    void Reset() noexcept;

private:
    std::weak_ptr<Transform> m_transform;
    std::size_t m_handlerId = 0;
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

    Matrix4 GetMatrix() const override;
    std::shared_ptr<Transform> Clone() const override;
    void AnimateTo(float scaleValue, float durationSeconds);

private:
    float m_scaleX;
    float m_scaleY;
    float m_centerX;
    float m_centerY;
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

    Matrix4 GetMatrix() const override;
    Matrix4 GetPositionMatrix() const override;
    bool HasRotation() const noexcept override;
    std::optional<float> TryGetRotation() const noexcept override;
    std::shared_ptr<Transform> Clone() const override;

private:
    float m_angle;
    float m_centerX;
    float m_centerY;
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
    ~TransformGroup() override;

    void AddChild(std::shared_ptr<Transform> transform);
    void ClearChildren();
    size_t GetChildCount() const;
    std::shared_ptr<Transform> GetChild(size_t index) const;

    Matrix4 GetMatrix() const override;
    Matrix4 GetPositionMatrix() const override;
    bool HasRotation() const noexcept override;
    std::optional<float> TryGetRotation() const noexcept override;
    std::shared_ptr<Transform> Clone() const override;

private:
    std::vector<std::shared_ptr<Transform>> m_children;
    std::vector<TransformSubscription> m_childChangedSubscriptions;
};

}

using Core::Matrix4;
using Core::Transform;
using Core::FloatAnimation;
using Core::ScaleTransform;
using Core::TranslateTransform;
using Core::RotateTransform;
using Core::MatrixTransform;
using Core::TransformGroup;

}
