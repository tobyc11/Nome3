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
    MainCamera->AddParent(RootNode);
}

}
