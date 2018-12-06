#pragma once

#include "Nome/IAppService.h"
#include "Scene/Scene.h"

namespace Nome
{

using tc::TAutoPtr;

class CEffiContext;
class CEffiCompiledPipeline;

class CDSLDemoCoreService : public IAppService
{
    DEFINE_APP_SERVICE_TYPE(CDSLDemoCoreService)

public:
protected:
    int Setup() override;
    int FrameUpdate() override;
    int Cleanup() override;

private:
    TAutoPtr<Scene::CScene> DemoScene;
	CEffiContext* EffiContext = nullptr;
	CEffiCompiledPipeline* CompiledPipeline = nullptr;
};

}
