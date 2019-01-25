#pragma once
#include <Matrix4.h>
#include <utility>
#include <set>
#include <memory>

namespace Nome
{

class CGraphicsDevice;
class CGeometry;
class CMaterial;
class CRenderer;
extern CRenderer* GRenderer;

class CBasicShader;
class CWireShader;
class CViewport;

class CRenderer
{
public:
	CRenderer();
	~CRenderer();

	CGraphicsDevice* GetGD() const { return GD; }

	void BeginView(const tc::Matrix4& view, const tc::Matrix4& proj, CViewport* viewport);
	void EndView();

    void Draw(const tc::Matrix4& modelMat, CGeometry* geometry, CMaterial* material);

	void Render();

private:
	CGraphicsDevice* GD;

    struct CViewData
    {
        tc::Matrix4 ViewMat;
        tc::Matrix4 ProjMat;
		CViewport* Viewport;

        struct CObjectData
        {
            CGeometry* Geom;
            CMaterial* Material;
            tc::Matrix4 ModelMat;

            bool operator<(const CObjectData& rhs) const { return Geom < rhs.Geom; }
        };

        std::set<CObjectData> DrawListBasic;
    };

    std::vector<CViewData> Views;

    std::unique_ptr<CBasicShader> BasicShader;
	std::unique_ptr<CWireShader> WireShader;
};

}
