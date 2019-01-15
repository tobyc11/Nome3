#pragma once
#include "Face.h"

#define _USE_MATH_DEFINES
#undef min
#undef max
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <map>
#include <set>
#include <utility>

typedef OpenMesh::PolyMesh_ArrayKernelT<> CNomeMesh;

namespace Nome::Scene
{

//Common base class for all mesh objects
class CMesh : public CEntity
{
	DEFINE_INPUT_ARRAY(CFace*, Faces) { MarkDirty(); }

public:
    using Super = CEntity;
    CMesh();
    CMesh(std::string name);

    void AddVertex(const std::string& name, Vector3 pos);

    bool HasVertex(const std::string& name) const
    {
        return NameToVert.find(name) != NameToVert.end();
    }

	Vector3 GetVertexPos(const std::string& name) const;
    void AddFace(const std::string& name, const std::vector<std::string>& facePoints);
    void ClearMesh();

	bool IsInstantiable() override;
	CEntity* Instantiate() override;

private:
	friend class CMeshInstance;

	CNomeMesh Mesh;
	std::map<std::string, CNomeMesh::VertexHandle> NameToVert;
	std::map<std::string, CNomeMesh::FaceHandle> NameToFace;
};

class CMeshInstance : public CEntity
{
public:
	using Super = CEntity;

	CMeshInstance(CMesh* generator);
	CMeshInstance(CMesh* generator, std::string name);

	void CopyFromGenerator();

private:
	TAutoPtr<CMesh> MeshGenerator;

	CNomeMesh Mesh;
	std::map<std::string, CNomeMesh::VertexHandle> NameToVert;
	std::map<std::string, CNomeMesh::FaceHandle> NameToFace;
};

}
