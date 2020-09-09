#include "Camera.h"

namespace Nome::Scene
{

void CCamera::CalculateProjMatrix() const
{
    if (!bProjMatrixDirty)
        return;

    ProjMatrix = Matrix4::ZERO;

    if (true /*!bIsOrthographic*/)
    {
        float h = 1.0f / tanf(FovY * tc::M_DEGTORAD_2);
        float w = h / AspectRatio;
        float q = FarClip / (NearClip - FarClip);
        float qn = NearClip * q;

        ProjMatrix[0][0] = w;
        ProjMatrix[1][1] = h;
        ProjMatrix[2][2] = q;
        ProjMatrix[2][3] = qn;
        ProjMatrix[3][2] = -1.0f;
    }
    else
    {
        throw std::runtime_error("Unimplemented feature.");
    }

    bProjMatrixDirty = false;
}

const Matrix4& CCamera::GetProjMatrix() const
{
    CalculateProjMatrix();
    return ProjMatrix;
}

Matrix4 CCamera::GetViewMatrix() const
{
    if (SceneTreeNode)
    {
        auto viewInv = SceneTreeNode->L2WTransform.GetValue(Matrix3x4::IDENTITY);
        return viewInv.Inverse().ToMatrix4();
    }
    return Matrix4::IDENTITY;
}

Frustum CCamera::GetFrustum() const
{
    auto frustum = Frustum();
    if (SceneTreeNode)
    {
        const auto& viewMatrix = SceneTreeNode->L2WTransform.GetValue(Matrix3x4::IDENTITY);

        frustum.Define(FovY, AspectRatio, 1.0f, NearClip, FarClip, viewMatrix);
        return frustum;
    }

    return frustum;
}

void COrbitCameraController::MouseMoved(int deltaX, int deltaY)
{
    if (bIsActive)
    {
        const float kCameraSpeed = 0.1f;
        Yaw += deltaX * kCameraSpeed;
        Pitch += deltaY * kCameraSpeed;
        Transform.MarkDirty();
    }
}

void COrbitCameraController::WheelMoved(int degree)
{
    const float kZoomSpeed = 1.0f / 15.0f * 0.25f;
    float delta = -degree * kZoomSpeed;
    if (Location.z + delta < 0.0f)
        Location.z /= 2.0f;
    else
        Location.z += delta;
    Transform.MarkDirty();
}

void COrbitCameraController::CalcTransform()
{
    auto rot = Quaternion(-Pitch, -Yaw, 0.0f);
    Transform.UpdateValue(Matrix3x4(rot.RotationMatrix())
                          * Matrix3x4(Location, Quaternion::IDENTITY, 1.0f));
}

}
