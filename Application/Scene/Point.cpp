#include "Point.h"

namespace Nome::Scene
{

void CPoint::MarkDirty()
{
	Super::MarkDirty();
	Point.MarkDirty();
}

void CPoint::UpdateEntity()
{
	Super::UpdateEntity();
	VI.Position = { X.GetValue(0.0f), Y.GetValue(0.0f), Z.GetValue(0.0f) };
	VI.Name = GetName();
	Point.UpdateValue(&VI);
	SetValid(true);
}

}
