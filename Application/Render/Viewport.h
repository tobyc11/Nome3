#pragma once
#include "SwapChain.h"

namespace Nome
{

class CViewportClient;

//Represents a window, widget, offscreen buffer, etc.
class CViewport
{
public:
	CViewport(CViewportClient* client) : Client(client) {}
	virtual ~CViewport() = default;

	virtual float GetAspectRatio() = 0;
	virtual float GetWidth() = 0;
	virtual float GetHeight() = 0;

	virtual void BindAndClear(ID3D11DeviceContext* ctx) = 0;

protected:
	CViewportClient* Client;
};

//Something that draws and possibly receives inputs from a viewport
class CViewportClient
{
public:
	virtual void Draw(CViewport* vp) = 0;

	virtual bool OnMousePress(CViewport* vp, uint32_t buttons, int x, int y) = 0;
	virtual bool OnMouseRelease(CViewport* vp, uint32_t buttons, int x, int y) = 0;
	virtual bool OnMouseMove(CViewport* vp, int x, int y) = 0;
	virtual bool OnMouseWheel(CViewport* vp, int degrees) = 0;
};

} /* namespace Nome */
