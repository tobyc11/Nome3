#pragma once
#include <Matrix4.h>
#include <Color.h>
#include <utility>
#include <set>
#include <memory>

namespace Nome
{

class CGraphicsDevice;
class CGeometry;
class CMaterial;
class CVertexBuffer;
class CRenderer;
extern CRenderer* GRenderer;

class CBasicShader;
class CWireShader;
class CPointShader;
class CPointPickingShader;
class CViewport;

struct CRendererPrivData;

struct CRenderViewInfo
{
    tc::Matrix4 ViewMat;
    tc::Matrix4 ProjMat;
    CViewport* Viewport;
    tc::Color ClearColor;
    float LineWidth;
    float PointRadius;

    struct Rect
    {
        int Left;
        int Top;
        int Right;
        int Bottom;
    };
    std::vector<Rect> ScissorRects;

    CRenderViewInfo& SetViewMat(tc::Matrix4 value) { ViewMat = std::move(value); return *this; }
    CRenderViewInfo& SetProjMat(tc::Matrix4 value) { ProjMat = std::move(value); return *this; }
    CRenderViewInfo& SetViewport(CViewport* value) { Viewport = std::move(value); return *this; }
    CRenderViewInfo& SetClearColor(tc::Color value) { ClearColor = std::move(value); return *this; }
    CRenderViewInfo& SetLineWidth(float value) { LineWidth = std::move(value); return *this; }
    CRenderViewInfo& SetPointRadius(float value) { PointRadius = std::move(value); return *this; }
};

/*
 * Needs:
 *   Faces (normal + color)
 *   Contour lines
 *   Edges (color)
 * How do I deal with material?
 */

class CRenderer
{
public:
    CRenderer();
    ~CRenderer();

    CGraphicsDevice* GetGD() const { return GD; }

    void BeginView(const CRenderViewInfo& viewInfo);
    void EndView();

    void Draw(const tc::Matrix4& modelMat, CGeometry* geometry, CMaterial* material);

    void Render();
    uint32_t RenderPickingPointsGetId(const CRenderViewInfo& viewInfo, CVertexBuffer* pointBuffer, uint32_t x, uint32_t y);

private:
    CGraphicsDevice* GD;

    struct CViewData : public CRenderViewInfo
    {
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
    std::unique_ptr<CPointShader> PointShader;
    std::unique_ptr<CPointPickingShader> PointPickingShader;

    CRendererPrivData* Pd;
};

}
