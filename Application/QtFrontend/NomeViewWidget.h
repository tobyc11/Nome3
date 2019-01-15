#pragma once
#include "D3DWidget.h"
#include "ImGuiImplQt.h"
#include <Render/SwapChain.h>
#include <QTimer>

namespace Nome
{

class CNomeViewWidget : public CD3DWidget
{
	Q_OBJECT

public:
	CNomeViewWidget(QWidget* parent);
	~CNomeViewWidget();

	void NewFrame();
	CSwapChain* GetSwapChain() const { return ViewSwapChain; }
	
protected:
	void resizeEvent(QResizeEvent* event) override
	{
		ViewSwapChain->ResizeBackbuffer(width(), height());
	}

private:
	CImGuiImplQt* ImGuiImpl;
	CSwapChain* ViewSwapChain;
};

}
