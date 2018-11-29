#pragma once

#include "SceneGraph.h"
#include "Camera.h"

namespace Nome::Scene
{

class CScene : public tc::FRefCounted
{
public:
    CScene();

    TAutoPtr<CSceneNode> GetRootNode() const
    {
        return RootNode;
    }

    TAutoPtr<CCamera> GetMainCamera() const
    {
        return MainCamera;
    }

    void CreateDefaultCamera();

private:
    TAutoPtr<CSceneNode> RootNode;
    TAutoPtr<CCamera> MainCamera;
};

}