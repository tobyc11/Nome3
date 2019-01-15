#include "Mesh.h"

namespace Nome::Scene
{

CMesh::CMesh()
{
}

CMesh::CMesh(std::string name) : CEntity(std::move(name))
{
}

void CMesh::AddVertex(const std::string& name, tc::Vector3 pos)
{
	CNomeMesh::VertexHandle vertex;
	vertex = Mesh.add_vertex(CNomeMesh::Point(pos.x, pos.y, pos.z));
	NameToVert.emplace(name, vertex);
}

Vector3 CMesh::GetVertexPos(const std::string& name) const
{
	auto iter = NameToVert.find(name);
	CNomeMesh::VertexHandle vertex = iter->second;
	const auto& pos = Mesh.point(vertex);
	return Vector3(pos[0], pos[1], pos[2]);
}

void CMesh::AddFace(const std::string& name, const std::vector<std::string>& facePoints)
{
	std::vector<CNomeMesh::VertexHandle> faceVHandles;
	for (const std::string& name : facePoints)
	{
		faceVHandles.push_back(NameToVert[name]);
	}
	auto faceHandle = Mesh.add_face(faceVHandles);
	NameToFace.emplace(name, faceHandle);
}

void CMesh::ClearMesh()
{
	Mesh.clear();
	NameToVert.clear();
}

bool CMesh::IsInstantiable()
{
	return true;
}

CEntity* CMesh::Instantiate()
{
	return new CMeshInstance(this);
}

CMeshInstance::CMeshInstance(CMesh* generator) : MeshGenerator(generator)
{
}

CMeshInstance::CMeshInstance(CMesh* generator, std::string name) : CEntity(name), MeshGenerator(generator)
{
}

void CMeshInstance::CopyFromGenerator()
{
	Mesh.clear();
	NameToVert.clear();
	NameToFace.clear();

	Mesh = MeshGenerator->Mesh;
	//TODO
}

}
