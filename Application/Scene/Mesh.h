#pragma once
#include "Face.h"

//We use OpenMesh for now. Can easily replace with in-house library when needed.
#define _USE_MATH_DEFINES
#undef min
#undef max
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <map>
#include <set>
#include <utility>

typedef OpenMesh::PolyMesh_ArrayKernelT<> CMeshImpl;

namespace Nome::Scene
{

class CMeshRenderPrivateData;

//Common base class for all mesh objects
class CMesh : public CEntity
{
	DEFINE_INPUT_ARRAY(CFace*, Faces) { MarkDirty(); }

public:
    using Super = CEntity;
    CMesh();
    CMesh(std::string name);

	void MarkDirty() override;
	void UpdateEntity() override;

	CMeshImpl::VertexHandle AddVertex(const std::string& name, Vector3 pos);

    bool HasVertex(const std::string& name) const
    {
        return NameToVert.find(name) != NameToVert.end();
    }

	Vector3 GetVertexPos(const std::string& name) const;
    void AddFace(const std::string& name, const std::vector<std::string>& facePoints);
	void AddFace(const std::string& name, const std::vector<CMeshImpl::VertexHandle>& facePoints);
	void AddLineStrip(const std::string& name, const std::vector<CMeshImpl::VertexHandle>& points);
    void ClearMesh();

	bool IsInstantiable() override;
	CEntity* Instantiate(CSceneTreeNode* treeNode) override;

private:
	friend class CMeshInstance;
	std::set<CMeshInstance*> InstanceSet;

	CMeshImpl Mesh;
	std::map<std::string, CMeshImpl::VertexHandle> NameToVert;
	std::map<std::string, CMeshImpl::FaceHandle> NameToFace;
	std::vector<CMeshImpl::VertexHandle> LineStrip;
};

class CMeshInstance : public CEntity
{
	DEFINE_OUTPUT_WITH_UPDATE(CMeshInstance*, SelectorSignal)
	{
		UpdateEntity();
		SelectorSignal.UpdateValue(this);
	}

	friend class CVertexSelector;

public:
	using Super = CEntity;

	CMeshInstance(CMesh* generator, CSceneTreeNode* stn);
	CMeshInstance(CMesh* generator, CSceneTreeNode* stn, std::string name);
	~CMeshInstance();

	void MarkDirty() override;
	void UpdateEntity() override;

	std::set<std::string>& GetFacesToDelete() { return FacesToDelete; }
	const std::set<std::string>& GetFacesToDelete() const { return FacesToDelete; }

	void Draw(CSceneTreeNode* treeNode);
	CVertexSelector* CreateVertexSelector(const std::string& name, const std::string& outputName);

	CSceneTreeNode* GetSceneTreeNode() const { return SceneTreeNode; }

	void CopyFromGenerator();

private:
	TAutoPtr<CMesh> MeshGenerator;
	///A weak pointer to the owning scene tree node
	CSceneTreeNode* SceneTreeNode;

	CMeshImpl Mesh;
	std::map<std::string, CMeshImpl::VertexHandle> NameToVert;
	std::map<std::string, CMeshImpl::FaceHandle> NameToFace;

	//Instance specific data
	std::set<std::string> FacesToDelete;

	CMeshRenderPrivateData* Priv;
};

class CVertexSelector : public Flow::CFlowNode
{
	DEFINE_INPUT(CMeshInstance*, MeshInstance)
	{
		Point.MarkDirty();
	}

	DEFINE_OUTPUT_WITH_UPDATE(CVertexInfo*, Point);

public:
	CVertexSelector(const std::string& targetName, const std::string& resultName) : TargetName(targetName)
	{
		VI.Name = resultName;
	}

private:
	std::string TargetName;

	CVertexInfo VI;
};

}
