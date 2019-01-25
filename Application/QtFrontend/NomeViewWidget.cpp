#include "NomeViewWidget.h"

namespace Nome
{

CNomeViewWidget::CNomeViewWidget(QWidget* parent, Scene::CScene* scene) : CD3DWidget(parent)
{
	setMouseTracking(true);
	HWND hWnd = (HWND) this->winId();
	ViewSwapChain = new CSwapChain(GRenderer, hWnd);
	ViewViewport = new CViewport(ViewSwapChain);
	ImGuiImpl = new CImGuiImplQt(this);

	//Create a camera controller and connect it to the scene
	OrbitCameraController = new Scene::COrbitCameraController();
	scene->ConnectCameraTransform(&OrbitCameraController->Transform);
}

CNomeViewWidget::~CNomeViewWidget()
{
	delete ImGuiImpl;
	delete ViewViewport;
	delete ViewSwapChain;
}

void CNomeViewWidget::NewFrame()
{
	ImGuiImpl->NewFrame();
}

void CNomeViewWidget::mousePressEvent(QMouseEvent* event)
{
	OrbitCameraController->Activate();
	LastX = event->x();
	LastY = event->y();
}

void CNomeViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
	OrbitCameraController->Inactivate();
}

void CNomeViewWidget::mouseMoveEvent(QMouseEvent* event)
{
	OrbitCameraController->MouseMoved(event->x() - LastX, event->y() - LastY);
	LastX = event->x();
	LastY = event->y();
}

void CNomeViewWidget::wheelEvent(QWheelEvent* event)
{
	QPoint numDegrees = event->angleDelta() / 8;
	OrbitCameraController->WheelMoved(numDegrees.y());
}

}
