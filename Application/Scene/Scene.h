#pragma once

#include "SceneGraph.h"
#include "Camera.h"

namespace Nome::Scene
{

class CScene
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

private:
    TAutoPtr<CSceneNode> RootNode;
    TAutoPtr<CCamera> MainCamera;
};

}