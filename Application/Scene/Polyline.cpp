#include "Polyline.h"

namespace Nome::Scene
{

void CPolyline::UpdateEntity()
{
    ClearMesh();
    Super::UpdateEntity();

    std::string nameOfLast;
    auto numPoints = Points.GetSize();
    for (size_t i = 0; i < numPoints; i++)
    {
        CVertexInfo* point = Points.GetValue(i, nullptr);
        AddVertex(point->Name, point->Position);
        if (i > 0)
        {
            AddFace("seg" + std::to_string(i - 1), {nameOfLast, point->Name});
        }
        nameOfLast = point->Name;
    }
}

}
