#pragma once

#include "SceneGraph.h"

#include <Frustum.h>

namespace Nome::Scene
{

using tc::Matrix4;
using tc::Frustum;

class CCamera : public tc::FRefCounted
{
public:
    CCamera(CSceneTreeNode* treeNode) : SceneTreeNode(treeNode) {}

    void CalculateProjMatrix() const;

    const Matrix4& GetProjMatrix() const;

    Matrix4 GetViewMatrix() const;

    Frustum GetFrustum() const;

    ///Property getter/setter
    float GetAspectRatio() const { return AspectRatio; }
    void SetAspectRatio(float value)
    {
        AspectRatio = value;
        bProjMatrixDirty = true;
    }

    float GetFovY() const { return FovY; }
    void SetFovY(float value)
    {
        FovY = value;
        bProjMatrixDirty = true;
    }

    float GetNearClip() const { return NearClip; }
    void SetNearClip(float value)
    {
        NearClip = value;
        bProjMatrixDirty = true;
    }

    float GetFarClip() const { return FarClip; }
    void SetFarClip(float value)
    {
        FarClip = value;
        bProjMatrixDirty = true;
    }

private:
    //This is where the view transform comes from
    TAutoPtr<CSceneTreeNode> SceneTreeNode;

    //bool bIsOrthographic = false;
    float AspectRatio = 1.0f;
    ///The vertical field of view, the default is 59 degrees
    float FovY = 59.0f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;
    //float Zoom;
    //float OrthoSize;

    mutable Matrix4 ProjMatrix;
    mutable bool bProjMatrixDirty = true;
};

class COrbitCameraController : public Flow::CFlowNode
{
    //A transform whose inverse is the view matrix
    DEFINE_OUTPUT_WITH_UPDATE(Matrix3x4, Transform)
    {
        CalcTransform();
    }

public:
    void Activate() { bIsActive = true; }
    void Inactivate() { bIsActive = false; }

    void MouseMoved(int deltaX, int deltaY);
    void WheelMoved(int degree);

private:
    void CalcTransform();

    bool bIsActive = false;

    Vector3 Location = { 0.0f, 0.0f, 10.0f };
    float Yaw = 0.0f, Pitch = 0.0f;
};

}
