#include "PolySoupMesh.h"

namespace Nome::Scene
{

CPolySoupMesh::CPolySoupMesh()
{

}

CPolySoupMesh::VertexHandle CPolySoupMesh::add_vertex(const tc::Vector3 &pos)
{
    VertexHandle handle = Vertices.size();
    Vertices.emplace_back(pos);
    return handle;
}

CPolySoupMesh::FaceHandle CPolySoupMesh::add_face(const std::vector<CPolySoupMesh::VertexHandle> &facePoints)
{
    FaceHandle handle = Faces.size();
    for (auto v : facePoints)
        if (v >= Vertices.size())
            return -1ull;
    Faces.emplace_back(facePoints, tc::Vector3(), false);
    return handle;
}

void CPolySoupMesh::delete_face(CPolySoupMesh::FaceHandle fh, bool placeHolder)
{
    Faces[fh].bIsDeleted = true;
}

void CPolySoupMesh::clear()
{
    Vertices.clear();
    Faces.clear();
}

std::vector<float> CPolySoupMesh::point(CPolySoupMesh::VertexHandle vh) const
{
    const auto& v = Vertices[vh];
    return {v.Position.x, v.Position.y, v.Position.z};
}

std::vector<float> CPolySoupMesh::normal(CPolySoupMesh::FaceHandle fh) const
{
    const auto& f = Faces[fh];
    return {f.Normal.x, f.Normal.y, f.Normal.z};
}

}
