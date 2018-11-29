#pragma once

#include "SceneGraph.h"

namespace Nome::Scene
{

using tc::Matrix4;

/*
 * Special scene node that also contains the view info
 */
class CCamera: public CSceneNode
{
public:
    void CalculateProjMatrix() const;

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
    //bool bIsOrthographic = false;
    float AspectRatio = 1.0f;
    ///The vertical field of view, the default is approx. 59 degrees
    float FovY = 1.02974f;
    float NearClip = 1.0f;
    float FarClip = 1000.0f;
    //float Zoom;
    //float OrthoSize;

    mutable Matrix4 ProjMatrix;
    mutable bool bProjMatrixDirty;
};

}
