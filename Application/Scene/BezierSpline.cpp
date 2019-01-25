#include "BezierSpline.h"

namespace Nome::Scene
{

void CBezierSpline::UpdateEntity()
{
	if (!IsDirty())
		return;

    Super::UpdateEntity();

	int n = (int)Segments.GetValue(6.0f);
	float radius = Radius.GetValue(1.0f);

	std::vector<CMeshImpl::VertexHandle> handles;
	for (int i = 0; i < n; i++)
	{
		float theta = (float)i / n * 2.f * (float)M_PI;
		handles.push_back(AddVertex("v" + std::to_string(i), { radius * cos(theta), radius *sin(theta), 0.0f }));
	}
	AddLineStrip("circle", handles);
}

}
