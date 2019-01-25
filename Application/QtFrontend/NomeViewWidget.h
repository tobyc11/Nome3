#pragma once
#include "D3DWidget.h"
#include "ImGuiImplQt.h"
#include <Render/Viewport.h>
#include <Scene/Scene.h>
#include <QTimer>

namespace Nome
{

class CNomeViewWidget : public CD3DWidget
{
	Q_OBJECT

public:
	CNomeViewWidget(QWidget* parent, Scene::CScene* scene);
	~CNomeViewWidget();

	void NewFrame();
	CSwapChain* GetSwapChain() const { return ViewSwapChain; }
	CViewport* GetViewport() const { return ViewViewport; }
	
protected:
	void resizeEvent(QResizeEvent* event) override
	{
		ViewSwapChain->ResizeBackbuffer(width(), height());
		ViewViewport->OnWindowResize(width(), height());
	}

	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

private:
	CImGuiImplQt* ImGuiImpl;
	CSwapChain* ViewSwapChain;
	CViewport* ViewViewport;
	tc::TAutoPtr<Scene::COrbitCameraController> OrbitCameraController;

	//For camera controller
	int LastX, LastY;
};

}
