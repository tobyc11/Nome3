#pragma once
#include "Face.h"
#include "Sharp.h"
#include "InteractivePoint.h"

#include <Ray.h>
#include <Plane.h> // Randy added on 10/10 for pick face
// We use OpenMesh for now. Can easily replace with in-house library when needed.
#define _USE_MATH_DEFINES
#undef min
#undef max
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>

#include <map>
#include <set>
#include <utility>

struct CNomeTraits : public OpenMesh::DefaultTraits
{
    // store barycenter of neighbors in this member
    VertexTraits
    {
    private:
        float  sharpness_;
    public:
        VertexT() : sharpness_( 0.0f ) { }
        [[nodiscard]] const float& sharpness() const { return sharpness_; }
        void set_sharpness(const float& _s) { sharpness_ = _s; }
    };
    EdgeTraits
    {
        private :
            float sharpness_;

        public:
            EdgeT() : sharpness_( 0.0f ) {}
            [[nodiscard]] const float& sharpness() const { return sharpness_; }
            void set_sharpness(const float& _s) { sharpness_ = _s; }

    };
};

typedef OpenMesh::PolyMesh_ArrayKernelT<CNomeTraits> CMeshImpl;

namespace Nome::Scene
{

class CMeshInstance;
class CVertexSelector;

// Common base class for all mesh objects
class CMesh : public CEntity
{
    DEFINE_INPUT_ARRAY(CFace*, Faces) { MarkDirty(); }
    DEFINE_INPUT_ARRAY(CSharp*, SharpPoints) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CMesh, CEntity);
    CMesh();
    explicit CMesh(std::string name);

    void MarkDirty() override;
    void UpdateEntity() override;
    void Draw(IDebugDraw* draw) override;

    CMeshImpl::VertexHandle AddVertex(const std::string& name, Vector3 pos, float sharpness = 0.0f);

    bool HasVertex(const std::string& name) const
    {
        return NameToVert.find(name) != NameToVert.end();
    }
    CMeshImpl::VertexHandle FindVertex(const std::string& name) const
    {
        return NameToVert.find(name)->second;
    }

    Vector3 GetVertexPos(const std::string& name) const;
    void AddPointSharpness(const CMeshImpl::VertexHandle& sharpPoint, float sharpness);
    void AddEdgeSharpness(const CMeshImpl::VertexHandle& e1, const CMeshImpl::VertexHandle& e2, float sharpness);
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
    std::map<CMeshImpl::FaceHandle, std::string> FaceToName; // Randy added
    std::map<std::vector<CMeshImpl::VertexHandle>, CMeshImpl::FaceHandle>
        FaceVertsToFace; // Randy added
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

    void RemoveFace(const std::vector<std::string>& facePoints); // Randy added, not fully implemented yet
    void PreserveFace(const std::vector<std::string>& facePoints); // Randy added, not fully implemented yet

    // I am really not sure whether this is a good interface or not
    const CMeshImpl& GetMeshImpl() const { return Mesh; }

    std::vector<std::pair<float, std::string>> PickVertices(const tc::Ray& localRay);
    std::vector<std::pair<float, std::string>> PickFaces(const tc::Ray& localRay); // Randy added on 10/10 to pick faces
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
    std::map<CMeshImpl::FaceHandle, std::string> FaceToName; // Randy added
    std::map<std::vector<CMeshImpl::VertexHandle>, CMeshImpl::FaceHandle>
        FaceVertsToFace; // Randy added
    bool AllVertSelected; // Randy added. Useful for knowing when the face has been selected
    // Instance specific data
    std::set<std::string> FacesToDelete;

    // std::map<std::string, std::pair<CMeshInstancePoint*, uint32_t>> PickingVerts; Randy commented out on 10/10 . I dont think it does anything???
    std::set<std::string> CurrSelectedVerts; // unique verts
    std::set<std::string> CurrSelectedVertNames; // a unique vert can be used in different meshes
                                                 // and have different names
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
