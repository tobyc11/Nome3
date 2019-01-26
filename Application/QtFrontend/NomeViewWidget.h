#pragma once
#include "D3DWidget.h"
#include "ImGuiImplQt.h"
#include <Render/Viewport.h>
#include <Scene/Scene.h>
#include <QTimer>

namespace Nome
{

class CNomeViewWidget : public CD3DWidget, public CViewport
{
	Q_OBJECT

public:
	CNomeViewWidget(QWidget* parent, CViewportClient* client);
	~CNomeViewWidget();

	CSwapChain* GetSwapChain() const { return SwapChain; }

	void UpdateAndDraw();

	// Inherited via CViewport
	float GetAspectRatio() override;
	float GetWidth() override;
	float GetHeight() override;
	void BindAndClear(ID3D11DeviceContext* ctx) override;
	
protected:
	void resizeEvent(QResizeEvent* event) override
	{
		SwapChain->ResizeBackbuffer(width(), height());
		RecreateDSV();
	}

	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

private:
	void RecreateDSV();

	CImGuiImplQt* ImGuiImpl;
	CSwapChain* SwapChain;

	ComPtr<ID3D11DepthStencilView> DepthBufferView;
};

}
