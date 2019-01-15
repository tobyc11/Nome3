#include "Face.h"

namespace Nome::Scene
{

void CFace::MarkDirty()
{
	Super::MarkDirty();
	Face.MarkDirty();
}

void CFace::UpdateEntity()
{
	Super::UpdateEntity();
}

size_t CFace::CountVertices() const
{
	return Points.GetSize();
}

}
