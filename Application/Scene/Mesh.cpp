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

    // Randy added below loop on 12/5. Do I need to add point->AddPointIntoMesh(this)?
    for (size_t i = 0; i < Points.GetSize(); i++)
    {
        auto* point = Points.GetValue(i, nullptr);
        this->AddVertex(point->Name, point->Position);
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
    VertToName.emplace(vertex, name); // Randy added on 10/15
    return vertex;
}

Vector3 CMesh::GetVertexPos(const std::string& name) const
{
    auto iter = NameToVert.find(name);
    CMeshImpl::VertexHandle vertex = iter->second;
    const auto& pos = Mesh.point(vertex);
    return Vector3(pos[0], pos[1], pos[2]);
}

void CMesh::AddFace(const std::string& name, const std::vector<std::string>& facePoints,
                    std::string faceSurfaceIdent)
{
    std::vector<CMeshImpl::VertexHandle> faceVHandles;
    for (const std::string& pointName : facePoints)
    {
        faceVHandles.push_back(NameToVert[pointName]);
    }

    AddFace(name, faceVHandles, faceSurfaceIdent);
}

void CMesh::AddFace(const std::string& name, const std::vector<CMeshImpl::VertexHandle>& facePoints,
                    std::string faceSurfaceIdent)
{
    auto faceHandle = Mesh.add_face(facePoints);

    // Randy added this on 12/12 for face entity coloring
    if (faceSurfaceIdent != "")
        fHWithColor.emplace(faceHandle, faceSurfaceIdent);

    if (!faceHandle.is_valid())
        printf("Could not add face %s into mesh %s\n", name.c_str(), GetName().c_str());
    FaceVertsToFace.emplace(facePoints,
                            faceHandle); // Key: vertex handle, Value: faceHandle. Randy added
    FaceToFaceVerts.emplace(faceHandle,
                            facePoints); // Key: vertex handle, Value: faceHandle. Randy added
    NameToFace.emplace(name, faceHandle);
    FaceToName.emplace(faceHandle, name); // Randy added
}

void CMesh::AddLineStrip(const std::string& name,
                         const std::vector<CMeshImpl::VertexHandle>& points)
{
    if (LineStrip.empty())
        LineStrip = points;
    else // Added this to allow line strips to be merged I think. RANDY TODO: this is not utilized
         // in merge currently. Implement a merge polyline command in the future
        LineStrip.insert(LineStrip.end(), points.begin(), points.end());
}

void CMesh::ClearMesh()
{
    Mesh.clear();
    NameToVert.clear();
    VertToName.clear();
    NameToFace.clear();
    FaceToName.clear(); // Randy added
    FaceVertsToFace.clear(); // Randy added
    FaceToFaceVerts.clear(); // Randy added
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
    Mesh.request_face_colors(); // Randy added on 10/10 for face selection. May be useful in the
                                // future for flat coloring. Currently doesn't do anything
    for (auto vH : Mesh.vertices())
    {
        Mesh.set_color(vH, { VERT_COLOR });
    }
    // don't need to set face color because it gets overwritten by material's instancecolor (shader)

    // IMPORTANT NOTE: This for loop is very messy. Fix this and RemoveFace later. If you're not
    // working on RemoveFace, feel free to ignore it for now
    for (const std::string& face : FacesToDelete)
    {
        auto iter = NameToFace.find(face);
        if (iter != NameToFace.end())
        {
            Mesh.delete_face(iter->second,
                             true); // TODO: Change to true to remove isolated vertices. But some
                                    // strange bug, where I can't delete a second mesh face occurs
            Mesh.garbage_collection(); // Needed to execute deleting the faces

            auto facehandle = NameToFace.at(face);
            auto faceverts = FaceToFaceVerts.at(facehandle);

            std::vector<int> deleted; // deleted indices
            int originalvertsize = VertToName.size();

            // useful for counting number of occurences
            std::vector<CMeshImpl::VertexHandle> vflattened;
            for (auto& imap : FaceToFaceVerts)
                vflattened.insert(
                    vflattened.begin(), imap.second.begin(),
                    imap.second
                        .end()); // https://stackoverflow.com/questions/313432/c-extend-a-vector-with-another-vector

            for (auto facevert : faceverts)
            { // facevert is not sorted by index
                bool isolated = std::count(vflattened.begin(), vflattened.end(), facevert)
                    == 1; // if this vertex is not shared by any other face
                if (isolated)
                    deleted.push_back(facevert.idx());
            }

            // map for new vert handle index to new vert handle
            std::map<int, CMeshImpl::VertexHandle> temp;
            for (auto newverthandle : Mesh.vertices())
            {
                int newindex = newverthandle.idx();
                std::cout << newindex << std::endl;
                temp.try_emplace(newindex, std::move(newverthandle));
            }
            // need a new map mapping old VertHandles to new verthandles
            std::map<CMeshImpl::VertexHandle, CMeshImpl::VertexHandle> tempVertToVert;
            int displacement = 0;

            // sort nametovert using sorted vector consisting of its key, value pairs
            std::vector<std::pair<std::string, CMeshImpl::VertexHandle>> v(NameToVert.begin(),
                                                                           NameToVert.end());
            sort(v.begin(), v.end(),
                 [=](std::pair<std::string, CMeshImpl::VertexHandle>& a,
                     std::pair<std::string, CMeshImpl::VertexHandle>& b) {
                     return a.second.idx() < b.second.idx();
                 });

            for (auto& pair : v)
            { // THE BUG IS THAT NAMETOVERT IS NOT SORTED
                int i = pair.second.idx();
                if (std::find(deleted.begin(), deleted.end(), i) == deleted.end())
                {
                    auto newhandle = temp.at(i - displacement);
                    tempVertToVert.try_emplace(std::move(pair.second), std::move(newhandle));
                }
                else // else,it is one that needs to be deleted. don't add it. but the next index i
                     // + 1, should correspond the new vert handle at index i
                    displacement += 1; // so we need to push vertices back
            }

            // handle both vertoname and nametovert together since they are the same size
            std::map<std::string, CMeshImpl::VertexHandle> newNameToVert;
            std::map<CMeshImpl::VertexHandle, std::string> newVertToName; // Randy added on 10/11

            // construct newVertToName
            for (auto& pair : VertToName)
            {
                if (tempVertToVert.find(pair.first)
                    != tempVertToVert.end()) // if this vert is not the one of the deleted ones
                {
                    auto updatedverthandle = tempVertToVert.at(pair.first);
                    newVertToName.emplace(updatedverthandle, pair.second);
                    newNameToVert.emplace(pair.second, updatedverthandle);
                }
            }

            // Maybe I need to clear them before reassigning?
            VertToName = newVertToName;
            NameToVert = newNameToVert;

            // map for new face handle index to new face handle
            std::map<int, CMeshImpl::FaceHandle> facetemp;
            for (auto newfacehandle : Mesh.faces())
                facetemp.try_emplace(newfacehandle.idx(), std::move(newfacehandle));

            std::vector<std::pair<std::string, CMeshImpl::FaceHandle>> f(NameToFace.begin(),
                                                                         NameToFace.end());
            sort(f.begin(), f.end(),
                 [=](std::pair<std::string, CMeshImpl::FaceHandle>& a,
                     std::pair<std::string, CMeshImpl::FaceHandle>& b) {
                     return a.second.idx() < b.second.idx();
                 });
            std::map<CMeshImpl::FaceHandle, CMeshImpl::FaceHandle> tempFaceToFace;
            int facedisplacement = 0;

            for (auto& pair : f)
            {
                int i = pair.second.idx();
                if (pair.first != face) // if the name is not the one that needs to be deleted
                {
                    auto newhandle = facetemp.at(i - facedisplacement);
                    tempFaceToFace.try_emplace(pair.second, newhandle);
                }
                else // this pair needs to be deleted, so we can take care of it here
                    facedisplacement += 1;
            }

            std::map<std::string, CMeshImpl::FaceHandle> newNameToFace;
            std::map<CMeshImpl::FaceHandle, std::string> newFaceToName; // Randy added on 10/11

            for (auto& pair : FaceToName)
            {
                if (pair.second != face) // if this face is not that one we want to delete
                {
                    auto updatedFacehandle = tempFaceToFace.at(pair.first);
                    newFaceToName.emplace(updatedFacehandle, pair.second);
                    newNameToFace.emplace(pair.second, updatedFacehandle);
                }
            }

            // Maybe I need to clear them before reassigning?
            FaceToName = newFaceToName;
            NameToFace = newNameToFace;

            //  Here, since none of them use Name, we can just rebuild it
            FaceToFaceVerts.clear();
            FaceVertsToFace.clear();
            for (auto& realface : Mesh.faces())
            {
                std::vector<CMeshImpl::VertexHandle> realverts;
                for (auto vert : Mesh.fv_range(realface))
                {
                    realverts.push_back(vert);
                }
                FaceToFaceVerts.emplace(realface, realverts);
                FaceVertsToFace.emplace(realverts, realface);
            }
        }
        else
        {
            printf("Couldn't find face %s for deletion in mesh instance %s\n", face.c_str(),
                   GetName().c_str());
        }
        MeshGenerator->Mesh = Mesh;
        MeshGenerator->NameToVert = NameToVert;
        MeshGenerator->VertToName = VertToName;
        MeshGenerator->NameToFace = NameToFace;
        MeshGenerator->FaceToName = FaceToName;
        MeshGenerator->FaceVertsToFace = FaceVertsToFace;
        MeshGenerator->FaceToFaceVerts = FaceToFaceVerts;
    }

    if (!Mesh.faces_empty())
    {
        Mesh.request_face_normals();
        Mesh.request_vertex_normals();
        Mesh.update_face_normals();
        Mesh.update_vertex_normals();
        // Update visual layer geometry
    }
    else
    {
        // std::cout << "no faces left in mesh. handle this else condition in the future" <<
        // std::endl; should probably implement this logic in the future OR else may be further bugs
        // cause this mesh has no faces after removing
    }
    FacesToDelete.clear(); // Randy added this on 10/18. Clear it, we have finished deleting them.

    // Randy commented the below section on 10/10. i don't think it does anything ???
    // Construct interactive points
    // CScene* scene = GetSceneTreeNode()->GetOwner()->GetScene();
    // for (auto pair : PickingVerts)
    //{
    //    scene->GetPickingMgr()->UnregisterObj(pair.second.second);
    //    delete pair.second.first;
    //}
    // PickingVerts.clear();

    // for (const auto& pair : NameToVert)
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
    FaceToFaceVerts.clear(); // Randy added

    Mesh = MeshGenerator->Mesh;
    // Since the handle only contains an index, we can just copy
    NameToVert = MeshGenerator->NameToVert;
    VertToName = MeshGenerator->VertToName; // Randy added on 10/15
    NameToFace = MeshGenerator->NameToFace;
    FaceToName = MeshGenerator->FaceToName; // Randy added
    FaceVertsToFace = MeshGenerator->FaceVertsToFace; // Randy added
    FaceToFaceVerts = MeshGenerator->FaceToFaceVerts; // Randy added

    CScene* scene = GetSceneTreeNode()->GetOwner()->GetScene();

    // Randy added on 12/12
    for (auto& pair : MeshGenerator->fHWithColor)
    {
        if (pair.second != "")
        {
            auto surfaceIdentifier = pair.second;
            auto surfaceEntity = scene->FindEntity(surfaceIdentifier);
            if (surfaceEntity)
            {
                CSurface* surfaceObject = dynamic_cast<CSurface*>(surfaceEntity.Get());
                std::array<float, 3> color = { surfaceObject->ColorR.GetValue(1.0f),
                                               surfaceObject->ColorG.GetValue(1.0f),
                                               surfaceObject->ColorB.GetValue(1.0f) };
                fHWithColorVector.emplace(pair.first, color);
            }
            else
            {
                std::cout << "Invalid surface color name" << std::endl;
            }
        }
    }
}

// Warning: Still buggy. Use with caution.
std::vector<std::string>
CMeshInstance::RemoveFace(const std::vector<std::string>& faceNames) // Randy added
{
    std::vector<std::string> removedVertName;
    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";

    for (auto faceName : faceNames)
    {
        // if faceName contains instPrefix, then the face is a part of this mesh instance
        if (faceName.find(instPrefix) != std::string::npos)
        {
            // the mesh's face name is the faceName - instprefix
            auto start_position_to_erase = faceName.find(instPrefix);
            auto meshfaceName = faceName.erase(start_position_to_erase, instPrefix.length());

            // if meshfaceName is actually a face within this mesh
            if (NameToFace.find(meshfaceName) != NameToFace.end())
            {
                std::cout << "found face, adding to faces to delete: " + meshfaceName << std::endl;
                FacesToDelete.insert(meshfaceName);
                this->MarkDirty();
                std::vector<std::string> temp;
                temp.push_back(meshfaceName);
                auto temp2 = GetFaceVertexNames(temp);
                removedVertName.insert(removedVertName.end(), temp2.begin(), temp2.end());
            }
        }
    }
    GetSceneTreeNode()->SetEntityUpdated(
        true); // Randy added this on 10/18. Didn't seem to fix bug?
    return removedVertName;
}

std::vector<std::pair<float, std::string>> CMeshInstance::PickFaces(const tc::Ray& localRay)
{
    std::vector<std::pair<float, std::string>> result;
    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
    for (const auto& pair : FaceVertsToFace)
    {
        auto points = pair.first;

        std::string facename = FaceToName.at(pair.second);
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

        // tc::Vector3 projected = localRay.Project(pos);
        // Randy note: They all have the same position because the local ray is transformed
        // based on the instance scene node

        auto testdist = localRay.HitDistance(*testplane);

        // WARNING: Doesn't give all combinations. Naive method only works with convex polygons
        // TODO: Optimize this to handle concave polygons
        std::vector<float> hitdistances;

        for (int i = 0; i < points.size(); i++)
        {
            for (int j = i + 1; j < points.size(); j++)
            {
                for (int k = j + 1; k < points.size(); k++)
                {
                    auto firstpoint = points[i];
                    auto secondpoint = points[j];
                    auto thirdpoint = points[k];
                    const auto& posArr1 = Mesh.point(firstpoint);
                    tc::Vector3 pos1 { posArr1[0], posArr1[1], posArr1[2] };
                    const auto& posArr2 = Mesh.point(secondpoint);
                    tc::Vector3 pos2 { posArr2[0], posArr2[1], posArr2[2] };
                    const auto& posArr3 = Mesh.point(thirdpoint);
                    tc::Vector3 pos3 { posArr3[0], posArr3[1], posArr3[2] };
                    auto testdist1 = localRay.HitDistance(pos1, pos2, pos3);
                    hitdistances.push_back(testdist1);
                }
            }
        }
        auto mindist = *std::min_element(hitdistances.begin(), hitdistances.end());
        std::cout << "Triangle hit distance:  " + std::to_string(mindist) << std::endl;
        auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
        if (mindist < 100)
        {
            result.emplace_back(mindist, instPrefix + facename);
        }
    }
    // std::sort(result.begin(), result.end());
    for (const auto& sel : result)
    {
        std::cout << "face added to pick list" << std::endl;
        printf("t=%.3f v=%s\n", sel.first, sel.second.c_str());
    }
    return result;
}

std::vector<std::pair<float, std::vector<std::string>>> CMeshInstance::PickPolylines(const tc::Ray& localRay)
{
    std::vector<std::pair<float, std::vector<std::string>>> result;

    auto meshClass = GetSceneTreeNode()->GetOwner()->GetEntity()->GetMetaObject().ClassName();
    auto meshName = GetSceneTreeNode()->GetOwner()->GetName();

    // If it's an already selected edge
    if (meshClass == "CPolyline" || meshClass == "CBSpline")
    {
        auto instPrefix = GetSceneTreeNode()->GetPath() + ".";

        std::vector<CMeshImpl::VertexHandle> edgeVertsOnly;
        for (const auto& pair : VertToName)
            edgeVertsOnly.push_back(pair.first);
        for (int i = 0; i < edgeVertsOnly.size(); i++) // adjacent elements are the edge verts
        {
            std::vector<float> hitdistances;
            std::map<float, std::vector<std::string>> distToNames;

            if (i == edgeVertsOnly.size() - 1)
            { // if at the last point, the last edge is last point connected to first point
                auto firstpoint = edgeVertsOnly[i];
                auto secondpoint = edgeVertsOnly[0];
                const auto& posArr1 = Mesh.point(firstpoint);
                const auto& posArr2 = Mesh.point(secondpoint);
                tc::Vector3 pos1 { posArr1[0], posArr1[1], posArr1[2] };
                tc::Vector3 pos2 { posArr2[0], posArr2[1], posArr2[2] };

                // naive method, extend the plane out of the line in 6 directions a little
                tc::Vector3 dummy1 = (pos1 + pos2) / 2;
                float offset = 0.170; // TODO: Optimize this to vary by scene
                float testdist1 = std::min(
                    { localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset }) });
                hitdistances.push_back(testdist1);
                std::cout << "testdist1a: " + std::to_string(testdist1) << std::endl;
                auto vertname1 = instPrefix + VertToName.at(firstpoint);
                auto vertname2 = instPrefix + VertToName.at(secondpoint);
                std::vector<std::string> names;
                names.push_back(vertname1);
                names.push_back(vertname2);
                distToNames.insert({ testdist1, names });
            }
            else
            {
                auto firstpoint = edgeVertsOnly[i];
                auto secondpoint = edgeVertsOnly[i + 1];
                const auto& posArr1 = Mesh.point(firstpoint);
                tc::Vector3 pos1 { posArr1[0], posArr1[1], posArr1[2] };
                const auto& posArr2 = Mesh.point(secondpoint);
                tc::Vector3 pos2 { posArr2[0], posArr2[1], posArr2[2] };

                // naive method, extend the plane out of the line in 6 directions a little
                tc::Vector3 dummy1 = (pos1 + pos2) / 2; 
                float offset = 0.170; // TODO: Optimize this to vary by scene
                float testdist1 = std::min(
                    { localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset }) });
                hitdistances.push_back(testdist1);
                std::cout << "testdist1b: " + std::to_string(testdist1) << std::endl;
                auto vertname1 = instPrefix + VertToName.at(firstpoint);
                auto vertname2 = instPrefix + VertToName.at(secondpoint);
                std::vector<std::string> names;
                names.push_back(vertname1);
                names.push_back(vertname2);
                distToNames.insert({ testdist1, names });
            }
            auto mindist = *std::min_element(hitdistances.begin(), hitdistances.end());
            std::cout << "Triangle hit distance for edge:  " + std::to_string(mindist) << std::endl;

            if (mindist < 100)
            {
                auto hitpointnames = distToNames.at(mindist); // Guaranteed to be two names
                auto name1withoutinstprefix = hitpointnames[0].substr(instPrefix.length());
                auto name2withoutinstprefix = hitpointnames[1].substr(instPrefix.length());

                auto point1 = NameToVert.at(name1withoutinstprefix);
                auto point2 = NameToVert.at(name2withoutinstprefix);

                GetSceneTreeNode()->GetOwner()->SelectNode();
                MarkDirty();
                

            }
        }
    }
        
    
    // std::sort(result.begin(), result.end());
    for (const auto& sel : result)
    {
        printf("t=%.3f v1=%s v2 =%s\n", sel.first, sel.second[0].c_str(), sel.second[1].c_str());
    }
    return result;
}

// Pick edges return a vector containing hit distance and the pair of edge vertex names
// NOTE: PickEdges is the buggiest among Pick functions
// Optimize in the future
std::vector<std::pair<float, std::vector<std::string>>>
CMeshInstance::PickEdges(const tc::Ray& localRay)
{
    std::vector<std::pair<float, std::vector<std::string>>> result;

    auto meshClass = GetSceneTreeNode()->GetOwner()->GetEntity()->GetMetaObject().ClassName();
    auto meshName = GetSceneTreeNode()->GetOwner()->GetName();

    // If it's an already selected edge
    if (meshClass == "CPolyline")
    {
        auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
        std::vector<CMeshImpl::VertexHandle> edgeVertsOnly;
        for (const auto& pair : VertToName)
            edgeVertsOnly.push_back(pair.first);
        std::vector<float> hitdistances;
        std::map<float, std::vector<std::string>> distToNames;
        auto firstpoint = edgeVertsOnly[0];
        auto secondpoint = edgeVertsOnly[1];
        const auto& posArr1 = Mesh.point(firstpoint);
        tc::Vector3 pos1 { posArr1[0], posArr1[1], posArr1[2] };
        const auto& posArr2 = Mesh.point(secondpoint);
        tc::Vector3 pos2 { posArr2[0], posArr2[1], posArr2[2] };
             
        // naive method, extend the plane out of the line in 6 directions a little
        tc::Vector3 dummy1 = (pos1 + pos2) / 2; 
        float offset = 0.170; // TODO: Optimize this to vary by scene
        float testdist1 = std::min(
            { localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
              localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0,  0 }),
              localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset}),
              localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
                       localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0 , 0}),
                       localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset }) });
        hitdistances.push_back(testdist1);
        std::cout << "testdist1b: " + std::to_string(testdist1) << std::endl;
        auto vertname1 = instPrefix + VertToName.at(firstpoint);
        auto vertname2 = instPrefix + VertToName.at(secondpoint);
        std::vector<std::string> names;
        names.push_back(vertname1);
        names.push_back(vertname2);
        distToNames.insert({ testdist1, names });
            
        auto mindist = *std::min_element(hitdistances.begin(), hitdistances.end());
        std::cout << "Triangle hit distance for edge:  " + std::to_string(mindist) << std::endl;

        if (mindist < 100)
        {
            auto hitpointnames = distToNames.at(mindist); // Guaranteed to be two names
            auto name1withoutinstprefix = hitpointnames[0].substr(instPrefix.length());
            auto name2withoutinstprefix = hitpointnames[1].substr(instPrefix.length());

            auto point1 = NameToVert.at(name1withoutinstprefix);
            auto point2 = NameToVert.at(name2withoutinstprefix);

            if (meshName.find("SELECTED") == std::string::npos)
            {
                GetSceneTreeNode()->GetOwner()->SelectNode();
                MarkDirty();
            }
            else // this is a temp selection polyline, deselect by adding to results
            {
                result.emplace_back(mindist, hitpointnames);
            }
        }
        
    }

    // Else, for entities other than Polyline or BSpline...
    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";

    for (const auto& pair :
         FaceVertsToFace) // if CPolyline or CBspline, this will silently get skipped
    {
        auto points = pair.first;

        std::string facename = FaceToName.at(pair.second);
        std::cout << "We are currently in " + GetSceneTreeNode()->GetPath() + "'s Pick Edges. "
                  << std::endl;
        std::cout << facename << std::endl;

        // WARNING: Doesn't give all combinations. Naive method only works with convex polygons
        std::vector<float> hitdistances;
        std::map<float, std::vector<std::string>> distToNames;
        // find which edge is intersected
        for (int i = 0; i < points.size(); i++)
        {
            if (i == points.size() - 1)
            { // if at the last point, the last edge is last point connected to first point
                auto firstpoint = points[i];
                auto secondpoint = points[0];
                const auto& posArr1 = Mesh.point(firstpoint);
                const auto& posArr2 = Mesh.point(secondpoint);
                tc::Vector3 pos1 { posArr1[0], posArr1[1], posArr1[2] };
                tc::Vector3 pos2 { posArr2[0], posArr2[1], posArr2[2] };

                // TODO: Fix this naive method. Naive method extend the plane out of the line in 6
                // directions
                tc::Vector3 dummy1 = (pos1 + pos2) / 2;
                float offset = 0.170; // TODO: Optimize this to vary by scene
                float testdist1 = std::min(
                    { localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset }) });
                hitdistances.push_back(testdist1);
                auto vertname1 = instPrefix + VertToName.at(firstpoint);
                auto vertname2 = instPrefix + VertToName.at(secondpoint);
                std::vector<std::string> names;
                names.push_back(vertname1);
                names.push_back(vertname2);
                distToNames.insert({ testdist1, names });
            }
            else
            {
                auto firstpoint = points[i];
                auto secondpoint = points[i+1];
                const auto& posArr1 = Mesh.point(firstpoint);
                tc::Vector3 pos1 { posArr1[0], posArr1[1], posArr1[2] };
                const auto& posArr2 = Mesh.point(secondpoint);
                tc::Vector3 pos2 { posArr2[0], posArr2[1], posArr2[2] };

                // TODO: Fix \this naive method. Naive method extend the plane out of the line in 6
                // directions
                tc::Vector3 dummy1 = (pos1 + pos2) / 2;
                float offset = 0.170; // TODO: Optimize this to vary by scene
                float testdist1 = std::min(
                    { localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, offset, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { offset, 0, 0 }),
                      localRay.HitDistance(pos1, pos2, dummy1 + Vector3 { 0, 0, offset }) });
                hitdistances.push_back(testdist1);
                auto vertname1 = instPrefix + VertToName.at(firstpoint);
                auto vertname2 = instPrefix + VertToName.at(secondpoint);
                std::vector<std::string> names;
                names.push_back(vertname1);
                names.push_back(vertname2);
                distToNames.insert({ testdist1, names });
            }
        }
        auto mindist = *std::min_element(hitdistances.begin(), hitdistances.end());
        std::cout << "Triangle hit distance for edge:  " + std::to_string(mindist) << std::endl;

        // This is outside of the for loop! incorrect!!
        if (mindist < 100)
        {
            auto hitpointnames = distToNames.at(mindist); // Guaranteed to be two names
            auto name1withoutinstprefix = hitpointnames[0].substr(instPrefix.length());
            auto name2withoutinstprefix = hitpointnames[1].substr(instPrefix.length());

            auto point1 = NameToVert.at(name1withoutinstprefix);
            auto point2 = NameToVert.at(name2withoutinstprefix);

            std::cout << mindist << std::endl;
            result.emplace_back(mindist, hitpointnames);
        }
    }
    // std::sort(result.begin(), result.end());
    for (const auto& sel : result)
    {
        printf("t=%.3f v1=%s v2 =%s\n", sel.first, sel.second[0].c_str(), sel.second[1].c_str());
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

std::vector<CMeshImpl::FaceHandle>
CMeshInstance::GetSelectedFaceHandles() // Randy added on 10/11 to assist with face coloring after
                                        // selection
{
    return CurrSelectedFaceHandles;
}

std::vector<std::string> CMeshInstance::GetFaceVertexNames(
    std::vector<std::string> facenames) // Randy added on 10/19 to return face vert names
{
    std::vector<std::string> vertnames;
    for (auto facename : facenames)
    {
        if (NameToFace.find(facename) != NameToFace.end())
        {
            auto tempfh = NameToFace[facename];
            auto temp1 = FaceToFaceVerts[tempfh];
            std::vector<std::string> temp2;
            for (auto vert : temp1)
            {
                auto vertname = VertToName[vert];
                auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
                auto instVertName = instPrefix + vertname;
                temp2.push_back(instVertName);
            }
            vertnames.insert(vertnames.end(), temp2.begin(), temp2.end());
        }
        else
        {
            vertnames.clear();
            return vertnames;
        }
    }

    return vertnames;
}

void CMeshInstance::MarkFaceAsSelected(const std::set<std::string>& faceNames, bool bSel)
{

    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";
    size_t prefixLen = instPrefix.length();
    for (const auto& name : faceNames)
    {
        auto iter = NameToFace.find(name.substr(prefixLen));
        if (iter == NameToFace.end())
            continue;

        auto handle = iter->second;
        const auto& original = Mesh.color(handle);
        printf("Before: %d %d %d\n", original[0], original[1], original[2]);
        auto iter0 = std::find(CurrSelectedFaceNamesWithPrefix.begin(),
                               CurrSelectedFaceNamesWithPrefix.end(), name);
        if (iter0 == CurrSelectedFaceNamesWithPrefix.end())
        {

            CurrSelectedFaceNames.push_back(name.substr(prefixLen));
            CurrSelectedFaceNamesWithPrefix.push_back(name);
            CurrSelectedFaceHandles.push_back(handle); // Randy added on 10/11 for face selection
        }
        else // it has already been selected, then deselect
        {
            auto iter1 = std::find(CurrSelectedFaceNames.begin(), CurrSelectedFaceNames.end(),
                                   name.substr(prefixLen));
            CurrSelectedFaceNames.erase(iter1); // Randy added this on 10/15
            auto iter2 = std::find(CurrSelectedFaceNamesWithPrefix.begin(),
                                   CurrSelectedFaceNamesWithPrefix.end(), name);
            CurrSelectedFaceNamesWithPrefix.erase(iter2);
            auto iter3 =
                std::find(CurrSelectedFaceHandles.begin(), CurrSelectedFaceHandles.end(), handle);
            CurrSelectedFaceHandles.erase(iter3); // Randy added on 10/11 for face selection
        }
    }
    GetSceneTreeNode()->SetEntityUpdated(true);
}

// TODO: Edge selection. Create Edge Handle data structures later.
void CMeshInstance::MarkEdgeAsSelected(const std::set<std::string>& vertNames, bool bSel)
{
    auto instPrefix = GetSceneTreeNode()->GetPath() + ".";

    // TODO 11/5 work on this, make sure the currselected stuff is actually working . also, fix
    // Nome3DView edge deselection
    std::vector<std::string> forEdge(vertNames.begin(), vertNames.end());
    auto name1withoutinstprefix = forEdge[0].substr(instPrefix.length());
    auto name2withoutinstprefix = forEdge[1].substr(instPrefix.length());
    auto point1 = NameToVert.at(name1withoutinstprefix);
    auto point2 = NameToVert.at(name2withoutinstprefix);
    // https://stackoverflow.com/questions/64243444/openmesh-find-edge-connecting-two-vertices
    CurrSelectedHalfEdgeHandles.push_back(
        Mesh.find_halfedge(point1, point2)); // not used yet, but can be used for sharpening edges
    ///////////////////////////////////////////////////////////////////

    size_t prefixLen = instPrefix.length();
    for (const auto& name : vertNames)
    {
        auto iter = NameToVert.find(name.substr(prefixLen));
        if (iter == NameToVert.end())
            continue;

        auto handle = iter->second;
        const auto& original = Mesh.color(handle);
        printf("Before: %d %d %d\n", original[0], original[1], original[2]);
        auto iter0 = std::find(CurrSelectedEdgeVertNamesWithPrefix.begin(),
                               CurrSelectedEdgeVertNamesWithPrefix.end(), name);
        if (iter0 == CurrSelectedEdgeVertNamesWithPrefix.end())
        {
            CurrSelectedEdgeVertNames.push_back(name.substr(prefixLen));
            CurrSelectedEdgeVertNamesWithPrefix.push_back(name);
            CurrSelectedEdgeVertHandles.push_back(handle);
        }
        else // it has already been selected, then deselect
        {
            auto iter1 = std::find(CurrSelectedEdgeVertNames.begin(),
                                   CurrSelectedEdgeVertNames.end(), name.substr(prefixLen));
            if (iter1 != CurrSelectedEdgeVertNames.end())
            { // erase once
                CurrSelectedEdgeVertNames.erase(iter1);
            }
            auto iter2 = std::find(CurrSelectedEdgeVertNamesWithPrefix.begin(),
                                   CurrSelectedEdgeVertNamesWithPrefix.end(), name);
            CurrSelectedEdgeVertNamesWithPrefix.erase(iter2);
            auto iter3 = std::find(CurrSelectedEdgeVertHandles.begin(),
                                   CurrSelectedEdgeVertHandles.end(), handle);
            CurrSelectedEdgeVertHandles.erase(iter3);
        }
    }
    GetSceneTreeNode()->SetEntityUpdated(true);
}

// Vertex selection
void CMeshInstance::MarkVertAsSelected(const std::set<std::string>& vertNames, bool bSel)
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
        if (std::find(CurrSelectedVertNamesWithPrefix.begin(),
                      CurrSelectedVertNamesWithPrefix.end(), name)
            == CurrSelectedVertNamesWithPrefix.end())
        { // if hasn't been selected before
            if (bSel)
                Mesh.set_color(handle, { VERT_SEL_COLOR });
            else
                Mesh.set_color(handle, { VERT_COLOR });
            CurrSelectedVertNames.push_back(name.substr(prefixLen));
            CurrSelectedVertNamesWithPrefix.push_back(name);
            CurrSelectedVertHandles.push_back(handle);
        }
        else // it has already been selected, then deselect
        {
            Mesh.set_color(handle, { VERT_COLOR });
            auto iter1 = std::find(CurrSelectedVertNames.begin(), CurrSelectedVertNames.end(),
                                   name.substr(prefixLen));
            if (iter1 != CurrSelectedVertNames.end())
            { // erase once
                CurrSelectedVertNames.erase(iter1);
            }
            auto iter2 = std::find(CurrSelectedVertNamesWithPrefix.begin(),
                                   CurrSelectedVertNamesWithPrefix.end(), name);
            CurrSelectedVertNamesWithPrefix.erase(iter2);
            auto iter3 =
                std::find(CurrSelectedVertHandles.begin(), CurrSelectedVertHandles.end(), handle);
            CurrSelectedVertHandles.erase(iter3);
        }
    }
    GetSceneTreeNode()->SetEntityUpdated(true);
}

void CMeshInstance::DeselectAll()
{
    for (const auto& name : CurrSelectedVertNames)
    {
        auto handle = NameToVert[name];
        Mesh.set_color(handle, { VERT_COLOR });
        GetSceneTreeNode()->SetEntityUpdated(true);
    }
    CurrSelectedVertNames.clear();
    CurrSelectedVertNamesWithPrefix.clear(); // added 10/3

    // added below on 10/10 for face deselection

    CurrSelectedFaceNames.clear();
    CurrSelectedFaceNamesWithPrefix.clear();
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
