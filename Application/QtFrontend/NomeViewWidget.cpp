#include "NomeViewWidget.h"

namespace Nome
{

CNomeViewWidget::CNomeViewWidget(QWidget* parent) : CD3DWidget(parent)
{
	setMouseTracking(true);
	HWND hWnd = (HWND) this->winId();
	ViewSwapChain = new CSwapChain(GRenderer, hWnd);
	ImGuiImpl = new CImGuiImplQt(this);
}

CNomeViewWidget::~CNomeViewWidget()
{
	delete ImGuiImpl;
	delete ViewSwapChain;
}

void CNomeViewWidget::NewFrame()
{
	ImGuiImpl->NewFrame();
}

}
