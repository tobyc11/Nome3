#pragma once
#include <Matrix4.h>

namespace Nome
{

class CGraphicsDevice;
class CGeometry;
class CMaterial;
class CRenderer;
extern CRenderer* GRenderer;

class CRenderer
{
public:
	CRenderer();
	~CRenderer();

	CGraphicsDevice* GetGD() const { return GD; }

	void BeginView(const tc::Matrix4& view, const tc::Matrix4& proj);
	void EndView();

	void Draw(CGeometry* geometry, CMaterial* material);

	void Render();

private:
	CGraphicsDevice* GD;
};

}
