#include "Face.h"
#include "Mesh.h"

namespace Nome::Scene
{

void CFace::MarkDirty()
{
	//Mark this entity dirty
	Super::MarkDirty();

	//And also mark the Face output dirty
	Face.MarkDirty();
}

void CFace::UpdateEntity()
{
	Super::UpdateEntity();
	Face.UpdateValue(this);
}

size_t CFace::CountVertices() const
{
	return Points.GetSize();
}

void CFace::AddFaceIntoMesh(CMesh* mesh) const
{
	std::vector<std::string> nameList;
	for (size_t i = 0; i < Points.GetSize(); i++)
	{
		auto* point = Points.GetValue(i, nullptr);
		mesh->AddVertex(point->Name, point->Position);
		nameList.push_back(point->Name);
	}
	mesh->AddFace(GetName(), nameList);
}

}
