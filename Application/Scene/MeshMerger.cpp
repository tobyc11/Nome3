#include "MeshMerger.h"
#include <unordered_map>

namespace Nome::Scene
{

inline static const float Epsilon = 0.01f; // 0.01f; 0.01f doesnt work with mobiusstrip test (1 1 0 500) endmobiusstrip 

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

    //OpenMesh::Subdivider::Uniform::CatmullClarkT<CMeshImpl>
    //    catmull; // https://www.graphics.rwth-aachen.de/media/openmesh_static/Documentations/OpenMesh-4.0-Documentation/a00020.html
    //// Execute 2 subdivision steps
    //CMeshImpl otherMesh = meshInstance.GetMeshImpl();
    //catmull.attach(otherMesh);
    //std::cout << "Apply catmullclark subdivision, may take a few minutes or so" << std::endl;
    //catmull(2);
    //catmull.detach();
    //auto tf = meshInstance.GetSceneTreeNode()->L2WTransform.GetValue(
    //    tc::Matrix3x4::IDENTITY); // The transformation matrix is the identity matrix by default
    //// Copy over all the vertices and check for overlapping
    //std::unordered_map<Vertex, Vertex> vertMap;
    //float maxY = -1 * std::numeric_limits<double>::infinity();
    //float minY = std::numeric_limits<double>::infinity();
    //for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end(); ++vi)
    //{
    //    std::cout << vi->idx() << std::endl;
    //    const auto& posArray = otherMesh.point(*vi);
    //    Vector3 localPos = Vector3(posArray[0], posArray[1], posArray[2]);
    //    Vector3 worldPos = tf * localPos;
    //    maxY = std::max(maxY, worldPos.y);
    //    minY = std::min(minY, worldPos.y);
    //}
    //for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end();
    //     ++vi) // Iterate through all the vertices in the mesh (the non-merger mesh, aka the one
    //           // you're trying copy vertices from)
    //{
    //    std::cout << vi->idx() << std::endl;
    //    const auto& posArray = otherMesh.point(*vi);
    //    Vector3 localPos = Vector3(posArray[0], posArray[1], posArray[2]);
    //    Vector3 worldPos = tf * localPos;
    //    /* Dont need since merged nodes have no overlapping vertices by construction from MergeIn
    //    auto [closestVert, distance] = FindClosestVertex(
    //        worldPos);
    //    if (distance < Epsilon)
    //    {
    //        vertMap[*vi] = closestVert;
    //    }*/
    //    // else

    //    // TODO: add vert into scene

    //    auto vnew =
    //        Mesh.add_vertex({ worldPos.x, worldPos.y,
    //                          worldPos.z }); // previous:  Mesh.add_vertex({ worldPos.x, worldPos.y
    //                                         // + (maxY - minY) + 10, worldPos.z});
    //    vertMap[*vi] = vnew;
    //    std::string vName = "v" + std::to_string(VertCount);
    //    NameToVert.insert({ vName, vnew });
    //    VertToName.insert({ vnew, vName });
    //    ++VertCount;
    //}

    //// Add faces
    //for (auto fi = otherMesh.faces_begin(); fi != otherMesh.faces_end(); ++fi)
    //{
    //    std::cout << fi->idx() << std::endl;
    //    std::vector<Vertex> verts;
    //    for (auto vert : otherMesh.fv_range(*fi))
    //        verts.emplace_back(vertMap[vert]);

    //    auto fnew = Mesh.add_face(verts);
    //    std::string fName = "v" + std::to_string(FaceCount);
    //    NameToFace.insert({ fName, fnew });
    //    FaceToName.insert({ fnew, fName });
    //    FaceCount++;
    //}
}

void CMeshMerger::MergeIn(const CMeshInstance& meshInstance)
{
    auto tf = meshInstance.GetSceneTreeNode()->L2WTransform.GetValue(
        tc::Matrix3x4::IDENTITY); // The transformation matrix is the identity matrix by default
    auto& otherMesh =
        meshInstance.GetDSMesh(); // Getting OpeshMesh implementation of a mesh. This allows us to
                                    // traverse the mesh's vertices/faces

    // Copy over all the vertices and check for overlapping
    std::unordered_map<Vertex*, Vertex*> vertMap;
    for (auto otherVert :  otherMesh.vertList) // Iterate through all the vertices in the mesh (the non-merger mesh, aka the one
               // you're trying copy vertices from)
    {
        Vector3 localPos = otherVert->position; // localPos is position before transformations
        Vector3 worldPos = tf * localPos; // worldPos is the actual position you see in the grid
        auto [closestVert, distance] = FindClosestVertex(worldPos); // Find closest vertex already IN MERGER mesh, not the actual mesh. This is
                       // to prevent adding two merger vertices in the same location!
        // As a side note, closestVert is a VertexHandle, which is essentially, a pointer to the
        // actual vertex. OpenMesh is great at working with these handles. You can basically treat
        // them as the vertex themselves.
        if (distance < Epsilon)
        { // this is to check for cases where there is an overlap (two vertices lie in the exact
          // same world space coordinate). We only want to create one merger vertex at this
          // location!
            vertMap[otherVert] = closestVert; // just set vi to the closestVert (which is a merger vertex
                                        // in the same location added in a previous iteration)
        }
        else // Else, we haven't added a vertex at this location yet. So lets add_vertex to the
             // merger mesh.
        {
            Vertex* copiedVert = new Vertex(worldPos.x, worldPos.y, worldPos.z, currMesh.nameToVert.size());
            copiedVert->name = "copiedVert" + std::to_string(currMesh.nameToVert.size()); // Randy this was causing the bug!!!!!!! the name was the same. so nameToVert remained size == 1
            currMesh.addVertex(copiedVert);
            vertMap[otherVert] = copiedVert; // Map actual mesh vertex to merged vertex.This dictionary is
                                 // useful for add face later.
            std::string vName = "v" + std::to_string(VertCount);
            ++VertCount; // VertCount is an attribute for this merger mesh. Starts at 0.
        }
    }

    // Add faces and create a face mesh for each
    for (auto otherFace : otherMesh.faceList) // Iterate through all the faces in the mesh (that is, the non-merger mesh, aka the
               // one you're trying to copy faces from)
    {
        std::vector<Vertex*> verts;
        for (auto vert : otherFace->vertices) //otherMesh vertices
        { // iterate through all the vertices on this face
            auto temp = vertMap[vert];
            verts.emplace_back(vertMap[vert]);
        } // Add the vertex handles
        //auto fnew =
        //Mesh.add_face(verts); // add_face processes the merger vertex handles and adds the face
                                  // into the merger mesh (Mesh refers to the merger mesh here)
        Face* copiedFace = new Face(verts);
        currMesh.addPolygonFace(verts);
        std::string fName = "v" + std::to_string(FaceCount);
        FaceCount++;
    }
    currMesh.buildBoundary();
    currMesh.computeNormals();

}

// Find closest vertex in current mesh's vertices
std::pair<Vertex*, float> CMeshMerger::FindClosestVertex(const tc::Vector3& pos)
{
    Vertex* result;
    float minDist = std::numeric_limits<float>::max();
    // TODO: linear search for the time being
    for (const auto& v : currMesh.vertList)
    {
        Vector3 pp = v->position;
        float dist = pos.DistanceToPoint(pp);
        if (dist < minDist)
        {
            minDist = dist;
            result = v;
        }
    }
    return { result, minDist };
    return { result, 99999 };
}

}