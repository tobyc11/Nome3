#pragma once
#include "Face.h"
#include "InteractivePoint.h"

#include <Ray.h>

// We use OpenMesh for now. Can easily replace with in-house library when needed.
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

class CMeshInstance;
class CVertexSelector;

// Common base class for all mesh objects
class CMesh : public CEntity
{
    DEFINE_INPUT_ARRAY(CFace*, Faces) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CMesh, CEntity);
    CMesh();
    explicit CMesh(std::string name);

    void MarkDirty() override;
    void UpdateEntity() override;
    void Draw(IDebugDraw* draw) override;

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

    void SetFromData(CMeshImpl mesh, std::map<std::string, CMeshImpl::VertexHandle> vnames,
                     std::map<std::string, CMeshImpl::FaceHandle> fnames);

    bool IsInstantiable() override;
    CEntity* Instantiate(CSceneTreeNode* treeNode) override;
    AST::ACommand* SyncToAST(AST::CASTContext& ctx, bool createNewNode) override;

private:
    friend class CMeshInstance;
    friend class CMeshMerger;
    std::set<CMeshInstance*> InstanceSet;

    CMeshImpl Mesh;
    std::map<std::string, CMeshImpl::VertexHandle> NameToVert;
    std::map<std::string, CMeshImpl::FaceHandle> NameToFace;
    std::vector<CMeshImpl::VertexHandle> LineStrip;
};

class CMeshInstancePoint : public CInteractivePoint
{
public:
    explicit CMeshInstancePoint(CMeshInstance* owner)
        : Owner(owner)
    {
    }

    [[nodiscard]] std::string GetPointPath() const;

private:
    CMeshInstance* Owner;
};

// This is the entity class for a mesh instance
// Why is this needed?
//  The same mesh might get instanciated multiple times and get separately edited by the user.
class CMeshInstance : public CEntity
{
    // This connects to all the vertex selectors naming a vertex from this mesh instance
    DEFINE_OUTPUT_WITH_UPDATE(CMeshInstance*, SelectorSignal)
    {
        UpdateEntity();
        SelectorSignal.UpdateValue(this);
    }

    friend class CVertexSelector;

public:
    using Super = CEntity;

    CMeshInstance(CMesh* generator, CSceneTreeNode* stn);
    ~CMeshInstance() override;

    // Called when the mesh entity is updated
    void MarkDirty() override;
    // Nothing changed about this mesh except its transformations and such
    void MarkOnlyDownstreamDirty();
    // Copy the actual mesh from the mesh entity and notify selectors
    void UpdateEntity() override;

    std::set<std::string>& GetFacesToDelete() { return FacesToDelete; }
    const std::set<std::string>& GetFacesToDelete() const { return FacesToDelete; }

    void Draw(IDebugDraw* draw) override;

    // Create a vertex selector with a vertex name, and a name for the resulting vertex
    CVertexSelector* CreateVertexSelector(const std::string& name, const std::string& outputName);

    // Get the scene tree node associated with this mesh instance
    CSceneTreeNode* GetSceneTreeNode() const { return SceneTreeNode; }

    void CopyFromGenerator();

    // I am really not sure whether this is a good interface or not
    const CMeshImpl& GetMeshImpl() const { return Mesh; }

    std::vector<std::pair<float, std::string>> PickVertices(const tc::Ray& localRay);
    void MarkAsSelected(const std::set<std::string>& vertNames, bool bSel);
    void DeselectAll();

private:
    TAutoPtr<CMesh> MeshGenerator;
    /// A weak pointer to the owning scene tree node
    CSceneTreeNode* SceneTreeNode;

    unsigned int TransformChangeConnection;

    CMeshImpl Mesh;
    std::map<std::string, CMeshImpl::VertexHandle> NameToVert;
    std::map<std::string, CMeshImpl::FaceHandle> NameToFace;

    // Instance specific data
    std::set<std::string> FacesToDelete;

    std::map<std::string, std::pair<CMeshInstancePoint*, uint32_t>> PickingVerts;
    std::set<std::string> CurrSelectedVerts;
};

class CVertexSelector : public Flow::CFlowNode
{
    DEFINE_INPUT(CMeshInstance*, MeshInstance) { Point.MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(CVertexInfo*, Point);

public:
    CVertexSelector(std::string targetName, const std::string& resultName)
        : TargetName(std::move(targetName))
    {
        VI.Name = resultName;
    }

    std::string GetPath() const;

private:
    std::string TargetName;

    CVertexInfo VI;
};

}
