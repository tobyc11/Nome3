#include "Polyline.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CPolyline)
{
    BindPositionalArgument(&CPolyline::Points, 1);
    BindNamedArgument(&CPolyline::bClosed, "closed", 0);
}

void CPolyline::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    std::vector<CMeshImpl::VertexHandle> vertArray;
    auto numPoints = Points.GetSize();
    CMeshImpl::VertexHandle firstVert;
    for (size_t i = 0; i < numPoints; i++)
    {
        CVertexInfo* point = Points.GetValue(i, nullptr);
        auto vertHandle = AddVertex(point->Name, point->Position);
        if (i == 0)
            firstVert = vertHandle;
        vertArray.push_back(vertHandle);
    }
    if (bClosed)
        vertArray.push_back(firstVert);
    AddLineStrip("polyline", vertArray);
}

void CPolyline::SetClosed(bool closed)
{
    bClosed = closed;
    MarkDirty();
}

}
