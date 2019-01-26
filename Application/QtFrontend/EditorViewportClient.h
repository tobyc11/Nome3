#pragma once
#include <Scene/Scene.h>
#include <Render/Viewport.h>

namespace Nome
{

class CEditorViewportClient : public CViewportClient
{
public:
	CEditorViewportClient(tc::TAutoPtr<Scene::CScene> scene);

	// Inherited via CViewportClient
	void Draw(CViewport* vp) override;

	bool OnMousePress(CViewport* vp, uint32_t buttons, int x, int y) override;
	bool OnMouseRelease(CViewport* vp, uint32_t buttons, int x, int y) override;
	bool OnMouseMove(CViewport* vp, int x, int y) override;
	bool OnMouseWheel(CViewport* vp, int degrees) override;

private:
	tc::TAutoPtr<Scene::CScene> Scene;

	//For camera controller
	tc::TAutoPtr<Scene::COrbitCameraController> OrbitCameraController;
	int LastX, LastY;
};

}
