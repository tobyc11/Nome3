#include "Mesh.h"
// Render related
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
#define VERT_SEL_COLOR 0, 255, 0

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

void CMesh::Draw(IDebugDraw* draw)
{
    CEntity::Draw(draw);

    if (!LineStrip.empty())
    {
        std::vector<Vector3> positions;
        for (auto vHandle : LineStrip)
        {
            const auto& vPos = Mesh.point(vHandle);
            positions.emplace_back(vPos[0], vPos[1], vPos[2]);
        }

        for (size_t i = 1; i < positions.size(); i++)
        {
            draw->LineSegment(positions[i - 1], positions[i]);
        }
    }
}

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
    FaceVertsToFace.emplace(facePoints,
                            faceHandle); // Key: vertex handle, Value: faceHandle. Randy Added
    NameToFace.emplace(name, faceHandle);
    FaceToName.emplace(faceHandle, name); // Randy added
}

void CMesh::AddLineStrip(const std::string& name,
                         const std::vector<CMeshImpl::VertexHandle>& points)
{
    LineStrip = points;
}

void CMesh::ClearMesh()
{
    Mesh.clear();
    NameToVert.clear();
    NameToFace.clear();
    FaceToName.clear(); // Randy added
    FaceVertsToFace.clear(); // Randy added
    LineStrip.clear();
}

// WARNING this function is not currently used and outdated. leaving here for future use
void CMesh::SetFromData(CMeshImpl mesh, std::map<std::string, CMeshImpl::VertexHandle> vnames,
                        std::map<std::string, CMeshImpl::FaceHandle> fnames)
{
    Mesh = std::move(mesh);
    NameToVert = std::move(vnames);
    NameToFace = std::move(fnames);
}

bool CMesh::IsInstantiable() { return true; }

CEntity* CMesh::Instantiate(CSceneTreeNode* treeNode) { return new CMeshInstance(this, treeNode); }

AST::ACommand* CMesh::SyncToAST(AST::CASTContext& ctx, bool createNewNode)
{
    if (!createNewNode)
        throw "unimplemented";
    auto* node = ctx.Make<AST::ACommand>(ctx.MakeToken("mesh"), ctx.MakeToken("endmesh"));
    node->PushPositionalArgument(ctx.MakeIdent(GetName()));
    size_t numFaces = Faces.GetSize();
    for (size_t i = 0; i < numFaces; i++)
    {
        this->GetName();
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

std::string CMeshInstancePoint::GetPointPath() const
{
    return Owner->GetSceneTreeNode()->GetPath() + "." + GetName();
}

CMeshInstance::CMeshInstance(CMesh* generator, CSceneTreeNode* stn)
    : MeshGenerator(generator) // MeshGenerator is the mesh the mesh instance refers to
    , SceneTreeNode(stn)
{
    SetName(tc::StringPrintf("_%s_%s", MeshGenerator->GetName().c_str(), GetName().c_str()));
    MeshGenerator->InstanceSet.insert(this);

    // We listen to the transformation changes of the associated tree node
    TransformChangeConnection = SceneTreeNode->OnTransformChange.Connect(
        std::bind(&CMeshInstance::MarkOnlyDownstreamDirty, this));
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
            printf("Couldn't find face %s for deletion in mesh instance %s\n", face.c_str(),
                   GetName().c_str());
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


    // Randy commented the below section on 10/10. i don't think it does anything ??? 
    // Construct interactive points
    //CScene* scene = GetSceneTreeNode()->GetOwner()->GetScene();
    //for (auto pair : PickingVerts)
    //{
    //    scene->GetPickingMgr()->UnregisterObj(pair.second.second);
    //    delete pair.second.first;
    //}
    //PickingVerts.clear();

    //for (const auto& pair : NameToVert)
    //{
    //    // Create a picking proxy point for each vertex
    //    CMeshInstancePoint* ip = new CMeshInstancePoint(this);
    //    ip->SetName(pair.first);
    //    const auto& p = Mesh.point(pair.second);
    //    Vector3 pos = { p[0], p[1], p[2] };
    //    ip->SetPosition(GetSceneTreeNode()->L2WTransform.GetValue(Matrix3x4::IDENTITY) * pos);
    //    // Register this picking point with the scene picking mgr
    //    uint32_t id = scene->GetPickingMgr()->RegisterObj(ip);
    //    PickingVerts.emplace(pair.first, std::make_pair(ip, id));
    //}

    Super::UpdateEntity();
    SetValid(MeshGenerator->IsEntityValid());
}

void CMeshInstance::Draw(IDebugDraw* draw) { MeshGenerator->Draw(draw); }

CVertexSelector* CMeshInstance::CreateVertexSelector(const std::string& name,
                                                     const std::string& outputName)
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
    FaceToName.clear(); // Randy added
    FaceVertsToFace.clear();

    Mesh = MeshGenerator->Mesh;
    // Since the handle only contains an index, we can just copy
    NameToVert = MeshGenerator->NameToVert;
    NameToFace = MeshGenerator->NameToFace;
    FaceToName = MeshGenerator->FaceToName; // Randy added
    FaceVertsToFace = MeshGenerator->FaceVertsToFace; // Randy added
}

void CMeshInstance::RemoveFace(const std::vector<std::string>& facePoints) // Randy added
{
    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
    if (AllVertSelected)
    {
        std::vector<CMeshImpl::VertexHandle> faceverthandles;
        for (const auto& myPair :
             NameToVert) // mesh vert name to vert handle. The issue is they all have the exact same
                         // mesh vert name and vert handle. Transformation is applied to each
        {
            auto test = myPair.first;
            std::cout << test << std::endl;
            std::cout << myPair.second.idx() << std::endl;
        }
        for (const auto& myPair : NameToFace) // mesh vert name to vert handle. The issue is they
                                              // all have the same mesh vert name and vert handle.
        {
            auto test = myPair.first;
            std::cout << test << std::endl;
            std::cout << myPair.second.idx() << std::endl;
        }
        for (auto name : facePoints)
        {

            // The next few lines fixed the bug
            std::cout << name << std::endl;
            auto suffix = name.substr(name.find_last_of(".")
                                      + 1); // get, for example, p5 from cube0.bottom.p5. THIS IS
                                            // CAUSING A BUG WHEN ALL FACES SHARE THE SAME SUFFIX
            auto prefix = name.substr(0, name.find_last_of(".") + 1);
            // auto it = NameToVert.find(suffix);  WRONG THIS WILL ALWAYTS BE FOUND
            if (prefix == instPrefix)
            {
                std::cout << "00" << std::endl;
                auto verthandle = NameToVert.at(suffix);
                faceverthandles.push_back(verthandle);
            }
        }
        CMeshImpl::FaceHandle faceHandle;
        std::string faceName = "none";
        for (const auto& myPair :
             FaceVertsToFace) // iterate  through all the faceverts and face pairs
        {
            auto currfaceverthandles = myPair.first;
            if (faceverthandles.size()
                != 0) // an empty vector is a permutation of any vector, so we need to ignore it
            {
                if (std::is_permutation(faceverthandles.begin(), faceverthandles.end(),
                                        currfaceverthandles.begin()))
                {
                    auto fhiter =
                        FaceVertsToFace.find(currfaceverthandles); // i think this is not finding
                    if (fhiter != FaceVertsToFace.end())
                    {
                        faceHandle = FaceVertsToFace.at(currfaceverthandles);
                    }
                    auto fniter = FaceToName.find(faceHandle);
                    if (fniter != FaceToName.end())
                    {
                        faceName = FaceToName.at(faceHandle);
                        FacesToDelete.insert(faceName);
                        this->MarkDirty(); // not sure if this is needed
                    }
                }
            }
        }
    }
    AllVertSelected = false; // done removing the face with all vert selected

    // if no faces added into facestodelete, this function silently fails
}

void CMeshInstance::PreserveFace(const std::vector<std::string>& facePoints) // Randy added
{
    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
    if (AllVertSelected)
    {
        std::vector<CMeshImpl::VertexHandle> faceverthandles;
        for (const auto& myPair : NameToVert)
        {
            auto test = myPair.first;
            std::cout << myPair.second.idx() << std::endl;
        }
        for (const auto& myPair : NameToFace)
        {
            auto test = myPair.first;
        }
        for (auto name : facePoints)
        {

            // The next few lines fixed the bug
            auto suffix = name.substr(name.find_last_of(".")
                                      + 1); // get, for example, p5 from cube0.bottom.p5. THIS IS
                                            // CAUSING A BUG WHEN ALL FACES SHARE THE SAME SUFFIX
            auto prefix = name.substr(0, name.find_last_of(".") + 1);
            //std::cout << prefix << std::endl;
            //std::cout << instPrefix << std::endl;
            // auto it = NameToVert.find(suffix);  WRONG THIS WILL ALWAYTS BE FOUND
            if (prefix == instPrefix)
            {
                auto verthandle = NameToVert.at(suffix);
                faceverthandles.push_back(verthandle);
            }
        }
        //std::cout << faceverthandles.size() << std::endl;
        //std::cout << "above is face vert handles size for preserved face" << std::endl;
        CMeshImpl::FaceHandle faceHandle;
        std::string faceName = "none";
        //std::cout << FaceVertsToFace.size() << std::endl;
        //std::cout << "above is total number of faces in mesh" << std::endl;
        for (const auto& myPair :
             FaceVertsToFace) // iterate  through all the faceverts and face pairs
        {

            auto currfaceverthandles = myPair.first;
            //std::cout << FaceVertsToFace.size() << std::endl;
            //std::cout << "above is total number of faces in mesh" << std::endl;
            //std::cout << currfaceverthandles.size() << std::endl; // THIS IS THE BUG
            //std::cout << "above is face vert handles size for preserved face" << std::endl;
            if (faceverthandles.size()
                != 0) // an empty vector is a permutation of any vector, so we need to ignore it
            {
                if (std::is_permutation(
                        faceverthandles.begin(), faceverthandles.end(),
                        currfaceverthandles
                            .begin())) // if the selected vertices match the current face vertices
                {

                    std::cout << "Found a permutation"
                              << std::endl;
                    auto fhiter =
                        FaceVertsToFace.find(currfaceverthandles); 
                    if (fhiter != FaceVertsToFace.end())
                    {
                        faceHandle = FaceVertsToFace.at(currfaceverthandles);
                    }
                    auto fniter = FaceToName.find(faceHandle);
                    if (fniter != FaceToName.end())
                    {

                        // This face is deleted here and was copied (with a new name) in
                        // TempMeshManager

                        faceName = FaceToName.at(faceHandle);
                        FacesToDelete.insert(faceName);
                        this->MarkDirty(); // not sure if this is needed
                    }
                }
            }
        }
    }
    AllVertSelected = false; // done removing the face with all vert selected

    // if no faces added into facestodelete, this function silently fails
}


std::vector<std::pair<float, std::string>> CMeshInstance::PickFaces(const tc::Ray& localRay) {

    // Randy look at this for delete face
    std::vector<std::pair<float, std::string>> result;
    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
    for (const auto& pair : FaceVertsToFace)
    {
        /// Construct from 3 vertices.
        /*Plane(const Vector3& v0, const Vector3& v1, const Vector3& v2) noexcept
        {
            Define(v0, v1, v2);
        }*/

        auto points = pair.first;
        auto firstpoint = points[0];
        auto secondpoint = points[1];
        auto thirdpoint = points[2];
        const auto& posArr1 = Mesh.point(firstpoint);
        tc::Vector3 pos1 { posArr1[0], posArr1[1], posArr1[2] };
        const auto& posArr2 = Mesh.point(secondpoint);
        tc::Vector3 pos2 { posArr2[0], posArr2[1], posArr2[2] };
        const auto& posArr3 = Mesh.point(thirdpoint);
        tc::Vector3 pos3 { posArr3[0], posArr3[1], posArr3[2] };
        auto testplane = new tc::Plane(pos1, pos2, pos3);
        auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
        std::cout << instPrefix << std::endl;
        std::cout << FaceToName.at(pair.second) << std::endl;
        std::cout << pos1.ToString() << std::endl;
        std::cout << pos2.ToString() << std::endl;
        std::cout << pos3.ToString() << std::endl;
       // tc::Vector3 projected = localRay.Project(pos);

       // bool Ray::InsideGeometry(const void* vertexData, unsigned vertexSize, unsigned vertexStart,
        //                         unsigned vertexCount) const


        // Randy note: They all have the same position because the local ray is transformed differently
        auto testdist = localRay.HitDistance(*testplane);
        std::cout << testdist << std::endl;
        //auto othertest = localRay.InsideGeometry(&points,4, 0, points.size()); 

       // std::cout << "other test: " + std::to_string(othertest) << std::endl;

        /// Return hit distance to a triangle, or infinity if no hit. Optionally return hit normal
        /// and hit barycentric coordinate at intersect point.
        //float HitDistance(const Vector3& v0, const Vector3& v1, const Vector3& v2,
          //                Vector3* outNormal = nullptr, Vector3* outBary = nullptr) const;

        auto testdist1 = localRay.HitDistance(pos1, pos2, pos3);
        
        std::cout << "other test #2: " + std::to_string(testdist1) << std::endl;
        //auto dist = (pos - projected).Length();
       
        //auto t = (localRay.Origin - projected).Length();
      

       
        
        //if (testdist < 10)//std::min(0.01f * t, 0.25f))
        //{
        //    //result.emplace_back(t, instPrefix + pair.first);
        //}




        //auto pos = (pos1 + pos2 + pos3) / 3.f; // find average face position
        //tc::Vector3 projected = localRay.Project(pos);
        //auto dist = (pos - projected).Length();
        //auto t = (localRay.Origin - projected).Length();
        //std::cout << dist << std::endl;
        //std::cout << "dist above" << std::endl;
        //if (dist < std::min(0.01f * t, 0.25f))
        //{
        //    //result.emplace_back(t, instPrefix + pair.first);
        //}
    }
    std::sort(result.begin(), result.end());

    for (const auto& sel : result)
    {
        printf("t=%.3f v=%s\n", sel.first, sel.second.c_str());
    }
    return result;


}



std::vector<std::pair<float, std::string>> CMeshInstance::PickVertices(const tc::Ray& localRay)
{
    // Randy look at this for delete face
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

        if (dist < std::min(0.01f * t, 0.25f))
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

        auto handle = iter->second;
        const auto& original = Mesh.color(handle);
        printf("Before: %d %d %d\n", original[0], original[1], original[2]);
        if (CurrSelectedVertNames.find(name) == CurrSelectedVertNames.end())
        { // if hasn't been selected before
            if (bSel)
                Mesh.set_color(handle, { VERT_SEL_COLOR });
            else
                Mesh.set_color(handle, { VERT_COLOR });
            CurrSelectedVerts.insert(name.substr(prefixLen));
            CurrSelectedVertNames.insert(name);

            if (CurrSelectedVertNames.size() == NameToVert.size())
            { // RANDY REMOVE THIS LATER if we selected all the available vertices on a face.
              // Reminder, a face is a mesh! A "mesh" command would be a collection of face meshes.
                AllVertSelected = true;
            }
        }
        else // it has already been selected, then deselect
        {
            Mesh.set_color(handle, { VERT_COLOR });
            if (CurrSelectedVerts.find(name.substr(prefixLen)) != CurrSelectedVerts.end())
            { // erase once
                CurrSelectedVerts.erase(name.substr(prefixLen));
            }

            CurrSelectedVertNames.erase(name);
        }
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
    CurrSelectedVertNames.clear(); // added 10/3
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
        printf("Vertex %s does not exist in entity %s\n", TargetName.c_str(),
               mi->GetName().c_str());
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