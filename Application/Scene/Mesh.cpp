#include "Mesh.h"
#include "RenderComponent.h"
#include "SceneGraph.h"
#include <StringPrintf.h>
#include <StringUtils.h>

namespace Nome::Scene
{

DEFINE_META_OBJECT(CMesh)
{
    // `mesh` command has no properties
    // `object` is currently unhandled
}

#define VERT_COLOR 255, 255, 255
#define VERT_SEL_COLOR 255, 255, 100

CMesh::CMesh() = default;

CMesh::CMesh(std::string name)
    : CEntity(std::move(name))
{
}

void CMesh::MarkDirty()
{
    if (IsDirty())
        return;

    Super::MarkDirty();

    for (auto* inst : InstanceSet)
        inst->MarkDirty();
}

void CMesh::UpdateEntity()
{
    if (!IsDirty())
        return;

    ClearMesh();
    bool isValid = true;
    for (size_t i = 0; i < Faces.GetSize(); i++)
    {
        // We assume the nullptr value is never returned, of course
        auto* face = Faces.GetValue(i, nullptr);
        bool successful = face->AddFaceIntoMesh(this);
        if (!successful)
        {
            isValid = false;
        }
    }

    Super::UpdateEntity();
    SetValid(isValid);
}

void CMesh::Draw(IDebugDraw* draw) { }

CMeshImpl::VertexHandle CMesh::AddVertex(const std::string& name, tc::Vector3 pos)
{
    // Silently fail if the name already exists
    auto iter = NameToVert.find(name);
    if (iter != NameToVert.end())
        return iter->second;

    CMeshImpl::VertexHandle vertex;
    vertex = Mesh.add_vertex(CMeshImpl::Point(pos.x, pos.y, pos.z));
    NameToVert.emplace(name, vertex);
    return vertex;
}

Vector3 CMesh::GetVertexPos(const std::string& name) const
{
    auto iter = NameToVert.find(name);
    CMeshImpl::VertexHandle vertex = iter->second;
    const auto& pos = Mesh.point(vertex);
    return Vector3(pos[0], pos[1], pos[2]);
}

void CMesh::AddFace(const std::string& name, const std::vector<std::string>& facePoints)
{
    std::vector<CMeshImpl::VertexHandle> faceVHandles;
    for (const std::string& pointName : facePoints)
    {
        faceVHandles.push_back(NameToVert[pointName]);
    }
    AddFace(name, faceVHandles);
}

void CMesh::AddFace(const std::string& name, const std::vector<CMeshImpl::VertexHandle>& facePoints)
{
    auto faceHandle = Mesh.add_face(facePoints);
    if (!faceHandle.is_valid())
        printf("Could not add face %s into mesh %s\n", name.c_str(), GetName().c_str());
    NameToFace.emplace(name, faceHandle);
}

void CMesh::AddLineStrip(const std::string& name, const std::vector<CMeshImpl::VertexHandle>& points)
{
    LineStrip = points;
}

void CMesh::ClearMesh()
{
    Mesh.clear();
    NameToVert.clear();
    NameToFace.clear();
    LineStrip.clear();
}

void CMesh::SetFromData(CMeshImpl mesh, std::map<std::string, CMeshImpl::VertexHandle> vnames,
                        std::map<std::string, CMeshImpl::FaceHandle> fnames)
{
    Mesh = std::move(mesh);
    NameToVert = std::move(vnames);
    NameToFace = std::move(fnames);
}

bool CMesh::IsInstantiable() { return true; }

CEntity* CMesh::Instantiate(CSceneTreeNode* treeNode)
{
    auto* meshInst = new CMeshInstance(this, treeNode);
    auto renderComp = std::make_shared<CMeshRenderComponent>();
    meshInst->AttachComponent(std::move(renderComp));
    return meshInst;
}

AST::ACommand* CMesh::SyncToAST(AST::CASTContext& ctx, bool createNewNode)
{
    if (!createNewNode)
        throw "unimplemented";
    auto* node = ctx.Make<AST::ACommand>(ctx.MakeToken("mesh"), ctx.MakeToken("endmesh"));
    node->PushPositionalArgument(ctx.MakeIdent(GetName()));
    size_t numFaces = Faces.GetSize();
    for (size_t i = 0; i < numFaces; i++)
    {
        auto* pFace = Faces.GetValue(i, nullptr);
        // Test whether the face is a sub-entity of mine
        //   For any sub-entity, we also serialize their AST
        //   Otherwise, this should have been an `object` command, which is unimplemented rn
        if (!tc::FStringUtils::StartsWith(pFace->GetName(), this->GetName()))
            throw std::runtime_error("Mesh's child faces corruption");

        node->AddSubCommand(pFace->MakeCommandNode(ctx, node));
    }
    return node;
}

std::string CMeshInstancePoint::GetPointPath() const { return Owner->GetSceneTreeNode()->GetPath() + "." + GetName(); }

CMeshInstance::CMeshInstance(CMesh* generator, CSceneTreeNode* stn)
    : MeshGenerator(generator)
    , SceneTreeNode(stn)
{
    SetName(tc::StringPrintf("_%s_%s", MeshGenerator->GetName().c_str(), GetName().c_str()));
    MeshGenerator->InstanceSet.insert(this);

    // We listen to the transformation changes of the associated tree node
    TransformChangeConnection =
        SceneTreeNode->OnTransformChange.Connect(std::bind(&CMeshInstance::MarkOnlyDownstreamDirty, this));
}

CMeshInstance::~CMeshInstance()
{
    // TODO: handle this circular reference stuff
    // SceneTreeNode->OnTransformChange.Disconnect(TransformChangeConnection);
    MeshGenerator->InstanceSet.erase(this);
}

void CMeshInstance::MarkDirty()
{
    Super::MarkDirty();
    SelectorSignal.MarkDirty();
}

void CMeshInstance::MarkOnlyDownstreamDirty() { SelectorSignal.MarkDirty(); }

void CMeshInstance::UpdateEntity()
{
    if (!IsDirty())
        return;
    MeshGenerator->UpdateEntity();
    CopyFromGenerator();
    Mesh.request_vertex_status();
    Mesh.request_vertex_colors();
    Mesh.request_edge_status();
    Mesh.request_face_status();
    for (auto vH : Mesh.vertices())
    {
        Mesh.set_color(vH, { VERT_COLOR });
    }
    for (const std::string& face : FacesToDelete)
    {
        auto iter = NameToFace.find(face);
        if (iter != NameToFace.end())
        {
            Mesh.delete_face(iter->second, false);
        }
        else
        {
            printf("Couldn't find face %s for deletion in mesh instance %s\n", face.c_str(), GetName().c_str());
        }
    }

    if (!Mesh.faces_empty())
    {
        Mesh.request_face_normals();
        Mesh.request_vertex_normals();
        Mesh.update_face_normals();
        Mesh.update_vertex_normals();
        // Update visual layer geometry
    }

    // Construct interactive points
    CScene* scene = GetSceneTreeNode()->GetOwner()->GetScene();
    for (auto pair : PickingVerts)
    {
        scene->GetPickingMgr()->UnregisterObj(pair.second.second);
        delete pair.second.first;
    }
    PickingVerts.clear();

    for (const auto& pair : NameToVert)
    {
        // Create a picking proxy point for each vertex
        CMeshInstancePoint* ip = new CMeshInstancePoint(this);
        ip->SetName(pair.first);
        const auto& p = Mesh.point(pair.second);
        Vector3 pos = { p[0], p[1], p[2] };
        ip->SetPosition(GetSceneTreeNode()->L2WTransform.GetValue(Matrix3x4::IDENTITY) * pos);
        // Register this picking point with the scene picking mgr
        uint32_t id = scene->GetPickingMgr()->RegisterObj(ip);
        PickingVerts.emplace(pair.first, std::make_pair(ip, id));
    }

    Super::UpdateEntity();
    SetValid(MeshGenerator->IsEntityValid());
}

void CMeshInstance::Draw(IDebugDraw* draw) { MeshGenerator->Draw(draw); }

CVertexSelector* CMeshInstance::CreateVertexSelector(const std::string& name, const std::string& outputName)
{
    auto* selector = new CVertexSelector(name, outputName);
    SelectorSignal.Connect(selector->MeshInstance);
    return selector;
}

void CMeshInstance::CopyFromGenerator()
{
    Mesh.clear();
    NameToVert.clear();
    NameToFace.clear();

    Mesh = MeshGenerator->Mesh;
    // Since the handle only contains an index, we can just copy
    NameToVert = MeshGenerator->NameToVert;
    NameToFace = MeshGenerator->NameToFace;
}

std::vector<std::pair<float, std::string>> CMeshInstance::PickVertices(const tc::Ray& localRay)
{
    std::vector<std::pair<float, std::string>> result;
    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
    for (const auto& pair : NameToVert)
    {
        const auto& posArr = Mesh.point(pair.second);
        assert(posArr.size() == 3);
        tc::Vector3 pos { posArr[0], posArr[1], posArr[2] };
        tc::Vector3 projected = localRay.Project(pos);
        auto dist = (pos - projected).Length();
        auto t = (localRay.Origin - projected).Length();
        if (dist < std::min(0.05f * t, 0.25f))
        {
            result.emplace_back(t, instPrefix + pair.first);
        }
    }
    std::sort(result.begin(), result.end());

    for (const auto& sel : result)
    {
        printf("t=%.3f v=%s\n", sel.first, sel.second.c_str());
    }
    return result;
}

void CMeshInstance::MarkAsSelected(const std::set<std::string>& vertNames, bool bSel)
{
    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
    size_t prefixLen = instPrefix.length();
    for (const auto& name : vertNames)
    {
        auto iter = NameToVert.find(name.substr(prefixLen));
        if (iter == NameToVert.end())
            continue;
        CurrSelectedVerts.insert(name.substr(prefixLen));
        auto handle = iter->second;
        const auto& original = Mesh.color(handle);
        printf("Before: %d %d %d\n", original[0], original[1], original[2]);
        if (bSel)
            Mesh.set_color(handle, { VERT_SEL_COLOR });
        else
            Mesh.set_color(handle, { VERT_COLOR });
    }
    GetSceneTreeNode()->SetEntityUpdated(true);
}

void CMeshInstance::DeselectAll()
{
    for (const auto& name : CurrSelectedVerts)
    {
        auto handle = NameToVert[name];
        Mesh.set_color(handle, { VERT_COLOR });
        GetSceneTreeNode()->SetEntityUpdated(true);
    }
    CurrSelectedVerts.clear();
}

void CVertexSelector::PointUpdate()
{
    // Assume MeshInstance is connected
    auto* mi = MeshInstance.GetValue(nullptr);
    if (!mi)
    {
        printf("Vertex %s does not have a mesh instance\n", TargetName.c_str());
        return;
    }
    auto iter = mi->NameToVert.find(TargetName);
    if (iter == mi->NameToVert.end())
    {
        printf("Vertex %s does not exist in entity %s\n", TargetName.c_str(), mi->GetName().c_str());
        return;
    }
    auto vertHandle = iter->second;
    const auto& p = mi->Mesh.point(vertHandle);
    VI.Position = { p[0], p[1], p[2] };
    VI.Position = mi->GetSceneTreeNode()->L2WTransform.GetValue(Matrix3x4::IDENTITY) * VI.Position;
    Point.UpdateValue(&VI);
}

std::string CVertexSelector::GetPath() const
{
    auto* mi = MeshInstance.GetValue(nullptr);
    if (!mi)
        throw std::runtime_error("Vertex selector cannot find its mesh instance");
    return mi->GetSceneTreeNode()->GetPath() + "." + TargetName;
}

}
