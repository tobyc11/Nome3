#include "MeshMerger.h"

#include <unordered_map>

namespace Nome::Scene
{

inline static const float Epsilon = 0.01f;

void CMeshMerger::UpdateEntity()
{
    if (!IsDirty())
        return;

    // Update is manual, so this entity has a dummy update method

    CEntity::UpdateEntity();
    SetValid(true);
}

void CMeshMerger::Catmull(const CMeshInstance& meshInstance)
{

    OpenMesh::Subdivider::Uniform::CatmullClarkT<CMeshImpl> catmull; // https://www.graphics.rwth-aachen.de/media/openmesh_static/Documentations/OpenMesh-4.0-Documentation/a00020.html
    // Execute 3 subdivision steps
    CMeshImpl otherMesh = meshInstance.GetMeshImpl();
    catmull.attach(otherMesh);
    std::cout << "Apply catmullclark subdivision, may take a minute or so" << std::endl;
    catmull(2);
    catmull.detach();
    auto tf = meshInstance.GetSceneTreeNode()->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY); // The transformation matrix is the identity matrix by default
    // Copy over all the vertices and check for overlapping
    std::unordered_map<CMeshImpl::VertexHandle, CMeshImpl::VertexHandle> vertMap;
    float maxY = -1 * std::numeric_limits<double>::infinity();
    std::cout << "inside catmull Complete, process vertices. Please wait." << std::endl;
    float minY = std::numeric_limits<double>::infinity();
    for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end(); ++vi)
    {
        std::cout << vi->idx() << std::endl;
        const auto& posArray = otherMesh.point(*vi);
        Vector3 localPos = Vector3(posArray[0], posArray[1], posArray[2]);
        Vector3 worldPos = tf * localPos;
        maxY = std::max(maxY, worldPos.y);
        minY = std::min(minY, worldPos.y);
    }
    for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end();
         ++vi) // Iterate through all the vertices in the mesh (the non-merger mesh, aka the one
               // you're trying copy vertices from)
    {
        std::cout << vi->idx() << std::endl;
        const auto& posArray = otherMesh.point(*vi);
        Vector3 localPos = Vector3(posArray[0], posArray[1],
                                   posArray[2]);
        Vector3 worldPos = tf * localPos; 
        /* Dont need since merged nodes have no overlapping vertices
        auto [closestVert, distance] = FindClosestVertex(
            worldPos); 
        if (distance < Epsilon)
        { 
            vertMap[*vi] = closestVert;
        }*/
        //else
        auto vnew = Mesh.add_vertex({ worldPos.x, worldPos.y + (maxY - minY) + 10, worldPos.z}); 
        vertMap[*vi] = vnew;
        std::string vName = "v" + std::to_string(VertCount); 
        NameToVert.insert({ vName, vnew }); 
        ++VertCount;
        
    }

    // Add faces
    for (auto fi = otherMesh.faces_begin(); fi != otherMesh.faces_end();
         ++fi) 
    {
        std::cout << fi->idx() << std::endl;
        std::vector<CMeshImpl::VertexHandle> verts;
        for (auto vert : otherMesh.fv_range(*fi))
            verts.emplace_back(vertMap[vert]); 
                                            
        auto fnew =
            Mesh.add_face(verts); 
        std::string fName = "v" + std::to_string(FaceCount);
        NameToFace.insert(
            { fName,
              fnew }); 
        FaceCount++;
    }
}

void CMeshMerger::MergeIn(const CMeshInstance& meshInstance)
{
    auto tf = meshInstance.GetSceneTreeNode()->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY); // The transformation matrix is the identity matrix by default
    const auto& otherMesh = meshInstance.GetMeshImpl(); // Getting OpeshMesh implementation of a mesh. This allows us to traverse the mesh's vertices/faces

    // Copy over all the vertices and check for overlapping
    std::unordered_map<CMeshImpl::VertexHandle, CMeshImpl::VertexHandle> vertMap;
    for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end(); ++vi) // Iterate through all the vertices in the mesh (the non-merger mesh, aka the one you're trying copy vertices from)
    {
        const auto& posArray = otherMesh.point(*vi);
        Vector3 localPos = Vector3(posArray[0], posArray[1], posArray[2]); // localPos is position before transformations (e.g. rotate, translate, etc.)
        Vector3 worldPos = tf * localPos; // worldPos is the actual position you see in the grid, after the transformation (e.g. rotate, translate, etc.)
        auto [closestVert, distance] = FindClosestVertex(worldPos); // Find closest vertex already IN MERGER mesh, not the actual mesh. This is to prevent adding two merger vertices in the same location!
        // As a side note, closestVert is a VertexHandle, which is essentially, a pointer to the actual vertex. OpenMesh is great at working with these handles. You can basically treat them as the vertex themselves.
        if (distance < Epsilon)
        { // this is to check for cases where there is an overlap (two vertices lie in the exact same world space coordinate). We only want to create one merger vertex at this location!
            vertMap[*vi] = closestVert; //just set vi to the closestVert (which is a merger vertex in the same location added in a previous iteration)
        }
        else // Else, we haven't added a vertex at this location yet. So lets add_vertex to the merger mesh.
        {
            auto vnew = Mesh.add_vertex({ worldPos.x, worldPos.y, worldPos.z }); // This adds a new vertex. Notice, we are passing in coordinates here, but it actually returns a vertex handle (essentially, a pointer to this vertex.
            vertMap[*vi] = vnew; // Map actual mesh vertex to merged vertex.This dictionary is useful for add face later.
            std::string vName = "v" + std::to_string(VertCount); // we of course need a name for this new vertex handle       
            NameToVert.insert({ vName, vnew }); // Add new merged vertex into NameToVert. This is if there wa sa floating point error above so we need to add an entirely new vertex + position ?
            ++VertCount; // VertCount is an attribute for this merger mesh. Starts at 0.
        }
    }

    // Add faces
    for (auto fi = otherMesh.faces_begin(); fi != otherMesh.faces_end(); ++fi) //Iterate through all the faces in the mesh (that is, the non-merger mesh, aka the one you're trying to copy faces from)
    {
        std::vector<CMeshImpl::VertexHandle> verts;
        for (auto vert : otherMesh.fv_range(*fi)) // iterate through all the vertices on this face 
            verts.emplace_back(vertMap[vert]); // Add the vertice handles from above. In most cases, it will match the actual mesh's? Unless there is a floating point precision error?
        auto fnew = Mesh.add_face(verts); // add_face processes the merger vertex handles and adds the face into the merger mesh (Mesh refers to the merger mesh here)
        std::string fName = "v" + std::to_string(FaceCount);
        NameToFace.insert({ fName, fnew }); // We add a new face in the same location as the actual mesh's face. This means if we adjust the actual mesh's parameters using a slider, you'll see the merger mesh in the actual mesh's original location
        FaceCount++;
    }
}

//Find closest vertex in current mesh's vertices
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
