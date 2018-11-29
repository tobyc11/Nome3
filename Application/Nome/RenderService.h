#pragma once

#include "IAppService.h"
#include "Scene/Scene.h"

namespace Nome
{

//A thin layer that wraps a Viewport
class CRenderService : public IAppService
{
    DEFINE_APP_SERVICE_TYPE_FLAGS(CRenderService, ASF_NONE)

public:
    Scene::CScene* GetScene() const;
    void SetScene(Scene::CScene* Scene);

protected:
    int Setup() override;

    int FrameUpdate() override
    {
        return 0;
    }

    int Cleanup() override;

    void Render() override;

private:
    Scene::CScene* Scene;
};

}
