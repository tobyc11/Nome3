#include "Mesh.h"
//Render related
#include "SceneGraph.h"
#include <Render/Material.h>
#include <Render/Geometry.h>
#include <Render/Renderer.h>
#include <StringPrintf.h>

namespace Nome::Scene
{

class CMeshRenderPrivateData
{
public:
    TAutoPtr<CStaticMeshGeometry> RenderGeometry;
    TAutoPtr<CStaticMeshGeometry> LineGeometry;
    TAutoPtr<CMaterial> Material;
};

CMesh::CMesh()
{
}

CMesh::CMesh(std::string name) : CEntity(std::move(name))
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
        //We assume the nullptr value is never returned, of course
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

CMeshImpl::VertexHandle CMesh::AddVertex(const std::string& name, tc::Vector3 pos)
{
    //Silently fail if the name already exists
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

bool CMesh::IsInstantiable()
{
    return true;
}

CEntity* CMesh::Instantiate(CSceneTreeNode* treeNode)
{
    return new CMeshInstance(this, treeNode);
}

CMeshInstance::CMeshInstance(CMesh* generator, CSceneTreeNode* stn)
    : MeshGenerator(generator), SceneTreeNode(stn), Priv(new CMeshRenderPrivateData())
{
    SetName(tc::StringPrintf("_%s_%s", MeshGenerator->GetName().c_str(), GetName().c_str()));
    MeshGenerator->InstanceSet.insert(this);

    //We listen to the transformation changes of the associated tree node
    TransformChangeConnection = SceneTreeNode->OnTransformChange.Connect(
        std::bind(&CMeshInstance::MarkOnlyDownstreamDirty, this));
}

CMeshInstance::~CMeshInstance()
{
    SceneTreeNode->OnTransformChange.Disconnect(TransformChangeConnection);
    MeshGenerator->InstanceSet.erase(this);
    delete Priv;
}

void CMeshInstance::MarkDirty()
{
    Super::MarkDirty();
    SelectorSignal.MarkDirty();
}

void CMeshInstance::MarkOnlyDownstreamDirty()
{
    SelectorSignal.MarkDirty();
}

void CMeshInstance::UpdateEntity()
{
    if (!IsDirty())
        return;
    MeshGenerator->UpdateEntity();
    CopyFromGenerator();
    Mesh.request_vertex_status();
    Mesh.request_edge_status();
    Mesh.request_face_status();
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

    if (!Priv->Material)
    {
        Priv->Material = new CMaterial();
    }

    if (!Mesh.faces_empty())
    {
        Mesh.request_face_normals();
        Mesh.request_vertex_normals();
        Mesh.update_face_normals();
        Mesh.update_vertex_normals();
        Priv->RenderGeometry = new CStaticMeshGeometry(Mesh);
    }
    else
        Priv->RenderGeometry = nullptr;

    //The upstream mesh contains a polyline of some sort
    if (!MeshGenerator->LineStrip.empty())
    {
        std::vector<Vector3> positions;
        for (auto vHandle : MeshGenerator->LineStrip)
        {
            const auto& vPos = MeshGenerator->Mesh.point(vHandle);
            positions.emplace_back(vPos[0], vPos[1], vPos[2]);
        }
        Priv->LineGeometry = new CStaticMeshGeometry(positions);
        Priv->LineGeometry->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    }

    Super::UpdateEntity();
    SetValid(MeshGenerator->IsEntityValid());
}

void CMeshInstance::Draw(CSceneTreeNode* treeNode)
{
    tc::Matrix4 modelMat = treeNode->L2WTransform.GetValue(Matrix3x4::IDENTITY).ToMatrix4();
    if (Priv->RenderGeometry)
    {
        GRenderer->Draw(modelMat, Priv->RenderGeometry, Priv->Material);
    }
    if (Priv->LineGeometry)
    {
        GRenderer->Draw(modelMat, Priv->LineGeometry, Priv->Material);
    }
}

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
    //Since the handle only contains an index, we can just copy
    NameToVert = MeshGenerator->NameToVert;
    NameToFace = MeshGenerator->NameToFace;
}

void CVertexSelector::PointUpdate()
{
    //Assume MeshInstance is connected
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

}
