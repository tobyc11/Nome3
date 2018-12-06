#pragma once

#include "Nome/IAppService.h"
#include "Scene/Camera.h"
#include "Mesh.h"

namespace Nome
{

class CDSLDemoRenderer : public IAppService
{
	DEFINE_APP_SERVICE_TYPE_FLAGS(CDSLDemoRenderer, ASF_RENDER)

public:
	void SetRenderMesh(CEffiMesh* mesh)
	{
		Mesh = mesh;
	}

	void SetCamera(Scene::CCamera* cam)
	{
		Camera = cam;
	}

protected:
	int Setup() override;
	int FrameUpdate() override;
	int Cleanup() override;

	void Render() override;

private:
	tc::TAutoPtr<Scene::CCamera> Camera;
	CEffiMesh* Mesh = nullptr;
};

}
