#include "Renderer.h"
#include "GraphicsDevice.h"
#include "Geometry.h"
#include "Material.h"

namespace Nome
{

CRenderer StaticRenderer;
CRenderer* GRenderer = &StaticRenderer;

CRenderer::CRenderer()
{
	GD = new CGraphicsDevice();
}

CRenderer::~CRenderer()
{
	delete GD;
}

void CRenderer::BeginView(const tc::Matrix4& view, const tc::Matrix4& proj)
{
}

void CRenderer::EndView()
{
}

void CRenderer::Draw(CGeometry* geometry, CMaterial* material)
{
}

void CRenderer::Render()
{
}

}
