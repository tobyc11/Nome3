#pragma once
#include "SceneGraph.h"
#include <Frustum.h>
#include <stdexcept>

namespace Nome::Scene
{

using tc::Frustum;
using tc::Matrix4;

class CLens
{
public:
    [[nodiscard]] Matrix4 CalculateProjMatrix() const;

    [[nodiscard]] float GetAspectRatio() const { return AspectRatio; }
    void SetAspectRatio(float value) { AspectRatio = value; }

    [[nodiscard]] float GetFovY() const { return FovY; }
    void SetFovY(float value) { FovY = value; }

    [[nodiscard]] float GetNearClip() const { return NearClip; }
    void SetNearClip(float value) { NearClip = value; }

    [[nodiscard]] float GetFarClip() const { return FarClip; }
    void SetFarClip(float value) { FarClip = value; }

private:
    bool bIsOrthographic = false;
    float AspectRatio = 1.0f;
    /// The vertical field of view, the default is 59 degrees
    float FovY = 59.0f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;
    // float Zoom;
    // float OrthoSize;
};

// Based on SimpleCADRender/Camera.cpp from my other project
class CCamera : public CLens
{
public:
    enum class EDir
    {
        Up,
        Down,
        Left,
        Right
    };

    [[nodiscard]] Matrix4 GetViewMatrix() const;
    [[nodiscard]] Frustum GetFrustum() const;
    [[nodiscard]] Vector3 GetPosition() const { return Position; }

    void Orbit(EDir dir, float angle);
    void Turn(EDir dir, float angle);
    void Move(Vector3 deltaPos);

private:
    // The camera looks at Pivot from Position
    Vector3 Pivot { 0.0f, 0.0f, 0.0f }, Position { 0.0f, 3.0f, 4.0f };
    Vector3 Up { 0.0f, 1.0f, 0.0f };
};

}
