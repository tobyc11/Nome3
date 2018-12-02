#include "Scene.h"

namespace Nome::Scene
{

CScene::CScene()
{
    RootNode = new CSceneNode("root", true);
}

void CScene::CreateDefaultCamera()
{
    MainCamera = new CCamera("default_camera");
    MainCamera->SetDefaultTransform({Vector3(0.0f, 0.0f, 10.0f), Quaternion::IDENTITY, Vector3::ONE});
    MainCamera->AddParent(RootNode);
}

}
