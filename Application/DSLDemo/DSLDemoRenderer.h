#pragma once

#include "Nome/IAppService.h"
#include "Scene/Camera.h"
#include "Mesh.h"

namespace Nome
{

class CMeshRenderer
{
public:
	CMeshRenderer() = default;
	CMeshRenderer(CEffiMesh* mesh) : Mesh(mesh) {}
	CMeshRenderer(CEffiUnindexedMesh* mesh) : UnidxMesh(mesh) {}

	void Render(const Scene::CCamera& camera);

private:
	CEffiMesh* Mesh = nullptr;
	CEffiUnindexedMesh* UnidxMesh = nullptr;
};

class CDSLDemoRenderer : public IAppService
{
	DEFINE_APP_SERVICE_TYPE_FLAGS(CDSLDemoRenderer, ASF_RENDER)

public:
	void ClearRenderMesh()
	{
		MeshRenderer = CMeshRenderer();
	}

	void SetRenderMesh(CMeshRenderer renderer)
	{
		MeshRenderer = renderer;
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
	CMeshRenderer MeshRenderer;
};

}
