#include "Polyline.h"

namespace Nome::Scene
{

void CPolyline::UpdateEntity()
{
	if (!IsDirty())
		return;

    Super::UpdateEntity();

	std::vector<CMeshImpl::VertexHandle> vertArray;
    auto numPoints = Points.GetSize();
    for (size_t i = 0; i < numPoints; i++)
    {
        CVertexInfo* point = Points.GetValue(i, nullptr);
		vertArray.push_back(AddVertex(point->Name, point->Position));
		AddLineStrip("polyline", vertArray);
    }
}

}
