#include "MeshMerger.h"

#include <unordered_map>

namespace Nome::Scene
{

inline static const float Epsilon = 0.01f;

void CMeshMerger::MergeIn(const CMeshInstance& meshInstance)
{
    auto tf = meshInstance.GetSceneTreeNode()->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
    const auto& otherMesh = meshInstance.GetMeshImpl();

    // Copy over all the vertices and check for overlapping
    std::unordered_map<CMeshImpl::VertexHandle, CMeshImpl::VertexHandle> vertMap;
    for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end(); ++vi)
    {
        const auto& posArray = otherMesh.point(*vi);
        Vector3 localPos = Vector3(posArray[0], posArray[1], posArray[2]);
        Vector3 worldPos = tf * localPos;
        auto [closestVert, distance] = FindClosestVertex(worldPos);
        if (distance < Epsilon)
            vertMap[*vi] = closestVert;
        else
        {
            auto vnew = Mesh.add_vertex({ worldPos.x, worldPos.y, worldPos.z });
            vertMap[*vi] = vnew;
            std::string vName = "v" + std::to_string(VertCount);
            NameToVert.insert({ vName, vnew });
            ++VertCount;
        }
    }

    // Add faces
    for (auto fi = otherMesh.faces_begin(); fi != otherMesh.faces_end(); ++fi)
    {
        std::vector<CMeshImpl::VertexHandle> verts;
        for (auto vert : otherMesh.fv_range(*fi))
            verts.emplace_back(vertMap[vert]);
        auto fnew = Mesh.add_face(verts);
        std::string fName = "v" + std::to_string(FaceCount);
        NameToFace.insert({ fName, fnew });
        FaceCount++;
    }
}

TAutoPtr<CEntity> CMeshMerger::GetResultMesh()
{
    TAutoPtr<CMesh> mesh = new CMesh("globalMerge");
    mesh->SetFromData(Mesh, NameToVert, NameToFace);
    mesh->MarkDirty();
    return tc::static_pointer_cast<CEntity>(mesh);
}

std::pair<CMeshImpl::VertexHandle, float> CMeshMerger::FindClosestVertex(const tc::Vector3& pos)
{
    CMeshImpl::VertexHandle result;
    float minDist = std::numeric_limits<float>::max();
    // TODO: linear search for the time being
    for (const auto& v : Mesh.vertices())
    {
        const auto& point = Mesh.point(v);
        Vector3 pp = Vector3(point[0], point[1], point[2]);
        float dist = pos.DistanceToPoint(pp);
        if (dist < minDist)
        {
            minDist = dist;
            result = v;
        }
    }
    return { result, minDist };
}

}
