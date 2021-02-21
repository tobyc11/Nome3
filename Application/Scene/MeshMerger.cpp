#include "MeshMerger.h"
#include "OffsetRefiner.h"
#include "Subdivision.h"

#include <unordered_map>

namespace Nome::Scene
{
DEFINE_META_OBJECT(CMeshMerger)
{
    BindNamedArgument(&CMeshMerger::Level, "sd_level", 0);
    BindNamedArgument(&CMeshMerger::Height, "height", 0);
    BindNamedArgument(&CMeshMerger::Width, "width", 0);
}

inline static const float Epsilon = 0.01f;

void CMeshMerger::UpdateEntity()
{
    if (!IsDirty())
        return;
    subdivisionLevel = Level.GetValue(0);

    Super::UpdateEntity();

    // Update is manual, so this entity has a dummy update method

    SetValid(true);
}

void CMeshMerger::Catmull()
{

    bool needSubdivision = subdivisionLevel != 0;
    bool needOffset = (Width.GetValue(0) != 0 || Height.GetValue(0) != 0);
    if ((!needSubdivision && !needOffset) || MergedMesh.vertList.size() == 0)//.vertices_empty()) Randy changed the commented out method
    {
        return;
    }
    //Mesh.clear();
    currMesh.clear(); // Randy added this


    // OpenMesh::Subdivider::Uniform::CatmullClarkT<CMeshImpl> catmull; //
    // https://www.graphics.rwth-aachen.de/media/openmesh_static/Documentations/OpenMesh-4.0-Documentation/a00020.html
    // Execute 2 subdivision steps
    DSMesh otherMesh = MergedMesh;
    // catmull.attach(otherMesh);
    // prepare(otherMesh);

    if (needSubdivision)
    {
        //subdivide(otherMesh, subdivisionLevel, isSharp); Randy commented this out for now. add back asap 
        std::cout << "Apply catmullclark subdivision, may take a few minutes or so" << std::endl;
    }
    if (needOffset)
    {
        offset(otherMesh);
        std::cout << "Apply offset, may take a few minutes or so" << std::endl;
    }
    // catmull(4);
    // cleanup(otherMesh);
    // catmull.detach();
    // auto tf =
    // dynamic_cast<Scene::CMeshInstance*>(this)->GetSceneTreeNode()->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
    

    // // The transformation matrix is the identity matrix by default
    //auto tf = tc::Matrix3x4::IDENTITY;
    //// Copy over all the vertices and check for overlapping
    //std::unordered_map<CMeshImpl::VertexHandle, CMeshImpl::VertexHandle> vertMap;
    //float maxY = -1 * std::numeric_limits<double>::infinity();
    //float minY = std::numeric_limits<double>::infinity();
    //for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end(); ++vi)
    //{
    //    //        std::cout << vi->idx() << std::endl;
    //    const auto& posArray = otherMesh.point(*vi);
    //    Vector3 localPos = Vector3(posArray[0], posArray[1], posArray[2]);
    //    Vector3 worldPos = tf * localPos;
    //    maxY = std::max(maxY, worldPos.y);
    //    minY = std::min(minY, worldPos.y);
    //}
    //for (auto vi = otherMesh.vertices_begin(); vi != otherMesh.vertices_end();
    //     ++vi) // Iterate through all the vertices in the mesh (the non-merger mesh, aka the one
    //// you're trying copy vertices from)
    //{
    //    //        std::cout << vi->idx() << std::endl;
    //    const auto& posArray = otherMesh.point(*vi);
    //    Vector3 localPos = Vector3(posArray[0], posArray[1], posArray[2]);
    //    Vector3 worldPos = tf * localPos;
    //    /* Dont need since merged nodes have no overlapping vertices
    //    auto [closestVert, distance] = FindClosestVertex(
    //        worldPos);
    //    if (distance < Epsilon)
    //    {
    //        vertMap[*vi] = closestVert;
    //    }*/
    //    // else
    //    auto vnew = Mesh.add_vertex({ worldPos.x, worldPos.y + (maxY - minY) + 10, worldPos.z });
    //    vertMap[*vi] = vnew;
    //    std::string vName = "v" + std::to_string(VertCount);
    //   // NameToVert.insert({ vName, vnew }); Project AddOffset
    //    ++VertCount;
    //}

    //// Add faces
    //for (auto fi = otherMesh.faces_begin(); fi != otherMesh.faces_end(); ++fi)
    //{
    //    //        std::cout << fi->idx() << std::endl;
    //    std::vector<CMeshImpl::VertexHandle> verts;
    //    for (auto vert : otherMesh.fv_range(*fi))
    //        verts.emplace_back(vertMap[vert]);

    //    auto fnew = Mesh.add_face(verts);
    //    std::string fName = "v" + std::to_string(FaceCount);
    //    //NameToFace.insert({ fName, fnew }); Project AddOffset
    //    FaceCount++;
    //}

    // Randy replaced above Mergin with below. This is bad code. We shouldnt copy logic from MergeIn here. Issue with how we're storing the transformation

    auto tf = tc::Matrix3x4::IDENTITY; // The transformation matrix is the identity matrix by default
    //auto& otherMesh = meshInstance.GetDSMesh(); // Getting OpeshMesh implementation of a mesh. This
                                                // allows us to traverse the mesh's vertices/faces
    //auto meshClass =
    //    meshInstance.GetSceneTreeNode()->GetOwner()->GetEntity()->GetMetaObject().ClassName();
    //if (meshClass == "CPolyline")
    //{
    //    std::cout << "found Polyline entity" << std::endl;
    //    return; // skip for now, dont merge polyline entities
    //}
    //if (meshClass == "CBSpline")
    //{
    //    std::cout << "found Bspline entity" << std::endl;
    //    return; // skip for now, dont merge polyline related entities
    //}

    // Copy over all the vertices and check for overlapping
    std::unordered_map<Vertex*, Vertex*> vertMap;
    for (auto otherVert :
         otherMesh.vertList) // Iterate through all the vertices in the mesh (the non-merger mesh,
                             // aka the one you're trying copy vertices from)
    {
        Vector3 localPos = otherVert->position; // localPos is position before transformations
        Vector3 worldPos = tf * localPos; // worldPos is the actual position you see in the grid
        auto [closestVert, distance] = FindClosestVertex(
            worldPos); // Find closest vertex already IN MERGER mesh, not the actual mesh. This is
                       // to prevent adding two merger vertices in the same location!

        if (distance < Epsilon)
        { // this is to check for cases where there is an overlap (two vertices lie in the exact
          // same world space coordinate). We only want to create one merger vertex at this
          // location!
            vertMap[otherVert] =
                closestVert; // just set vi to the closestVert (which is a merger vertex
                             // in the same location added in a previous iteration)
        }
        else // Else, we haven't added a vertex at this location yet. So lets add_vertex to the
             // merger mesh.
        {
            Vertex* copiedVert =
                new Vertex(worldPos.x, worldPos.y, worldPos.z, currMesh.nameToVert.size());
            copiedVert->name =
                "copiedVert"
                + std::to_string(
                    currMesh.nameToVert.size()); // Randy this was causing the bug!!!!!!! the name
                                                 // was the same. so nameToVert remained size == 1
            currMesh.addVertex(copiedVert);
            vertMap[otherVert] = copiedVert; // Map actual mesh vertex to merged vertex.This
                                             // dictionary is useful for add face later.
            std::string vName = "v" + std::to_string(VertCount);
            ++VertCount; // VertCount is an attribute for this merger mesh. Starts at 0.
        }
    }

    // Add faces and create a face mesh for each
    for (auto otherFace :
         otherMesh.faceList) // Iterate through all the faces in the mesh (that is, the non-merger
                             // mesh, aka the one you're trying to copy faces from)
    {
        std::vector<Vertex*> verts;
        for (auto vert : otherFace->vertices) // otherMesh vertices
        { // iterate through all the vertices on this face
            auto temp = vertMap[vert];
            verts.emplace_back(vertMap[vert]);
        } // Add the vertex handles
        // auto fnew =
        // Mesh.add_face(verts); // add_face processes the merger vertex handles and adds the face
        // into the merger mesh (Mesh refers to the merger mesh here)
        Face* copiedFace = new Face(verts);
        currMesh.addPolygonFace(verts);
        std::string fName = "v" + std::to_string(FaceCount);
        FaceCount++;
    }
    currMesh.buildBoundary();
    currMesh.computeNormals();
}

void CMeshMerger::MergeClear()
{
    MergedMesh.clear();
    Mesh.clear();
}

void CMeshMerger::MergeIn(const CMeshInstance& meshInstance)
{
    auto tf = meshInstance.GetSceneTreeNode()->L2WTransform.GetValue(
        tc::Matrix3x4::IDENTITY); // The transformation matrix is the identity matrix by default
    auto& otherMesh = meshInstance.GetDSMesh(); // Getting OpeshMesh implementation of a mesh. This
                                                // allows us to traverse the mesh's vertices/faces

    auto meshClass =
        meshInstance.GetSceneTreeNode()->GetOwner()->GetEntity()->GetMetaObject().ClassName();
    if (meshClass == "CPolyline")
    {
        std::cout << "found Polyline entity" << std::endl;
        return; // skip for now, dont merge polyline entities
    }
    if (meshClass == "CBSpline")
    {
        std::cout << "found Bspline entity" << std::endl;
        return; // skip for now, dont merge polyline related entities
    }

    // Copy over all the vertices and check for overlapping
    std::unordered_map<Vertex*, Vertex*> vertMap;
    for (auto otherVert :
         otherMesh.vertList) // Iterate through all the vertices in the mesh (the non-merger mesh,
                             // aka the one you're trying copy vertices from)
    {
        Vector3 localPos = otherVert->position; // localPos is position before transformations
        Vector3 worldPos = tf * localPos; // worldPos is the actual position you see in the grid
        auto [closestVert, distance] = FindClosestVertex(
            worldPos); // Find closest vertex already IN MERGER mesh, not the actual mesh. This is
                       // to prevent adding two merger vertices in the same location!

        if (distance < Epsilon)
        { // this is to check for cases where there is an overlap (two vertices lie in the exact
          // same world space coordinate). We only want to create one merger vertex at this
          // location!
            vertMap[otherVert] =
                closestVert; // just set vi to the closestVert (which is a merger vertex
                             // in the same location added in a previous iteration)
        }
        else // Else, we haven't added a vertex at this location yet. So lets add_vertex to the
             // merger mesh.
        {
            Vertex* copiedVert =
                new Vertex(worldPos.x, worldPos.y, worldPos.z, currMesh.nameToVert.size());
            copiedVert->name = "copiedVert" + std::to_string(currMesh.nameToVert.size()); // Randy this was causing the bug!!!!!!! the name
                                                 // was the same. so nameToVert remained size == 1
            currMesh.addVertex(copiedVert);
            vertMap[otherVert] = copiedVert; // Map actual mesh vertex to merged vertex.This
                                             // dictionary is useful for add face later.
            std::string vName = "v" + std::to_string(VertCount);
            ++VertCount; // VertCount is an attribute for this merger mesh. Starts at 0.
        }
    }

    // Add faces and create a face mesh for each
    for (auto otherFace :
         otherMesh.faceList) // Iterate through all the faces in the mesh (that is, the non-merger
                             // mesh, aka the one you're trying to copy faces from)
    {
        std::vector<Vertex*> verts;
        for (auto vert : otherFace->vertices) // otherMesh vertices
        { // iterate through all the vertices on this face
            auto temp = vertMap[vert];
            verts.emplace_back(vertMap[vert]);
        } // Add the vertex handles
        // auto fnew =
        // Mesh.add_face(verts); // add_face processes the merger vertex handles and adds the face
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
}


// offset only added here
// Randy changed it to use DSMesh
bool CMeshMerger::offset(DSMesh & _m)
{
    float height = Height.GetValue(0.0f);
    float width = Width.GetValue(0.0f);
    if (height <= 0 && width <= 0)
    {
        return true;
    }

    COffsetRefiner offsetRefiner(_m, offsetFlag);
    offsetRefiner.Refine(height, width);
    _m.clear();

    std::vector<Vector3> vertices = offsetRefiner.GetVertices();
    std::vector<std::vector<int>> faces = offsetRefiner.GetFaces();

    // Print vertices
    printf("============ output vertices ======\n");
    for (int index = 0; index < vertices.size(); index++)
    {
        Vector3 point = vertices[index];
        //_m.add_vertex(CMeshImpl::Point(point.x, point.y, point.z));
        
        // Randy replaced above line with below two lines;
        Vertex* newVert = new Vertex(point.x, point.y, point.z, currMesh.vertList.size());
        _m.addVertex(newVert);
        
        printf("v%d: %f %f %f\n", index, point.x, point.y, point.z);
    }

    // Print faces
    printf("============ output faces ======\n");
    for (int index = 0; index < faces.size(); index++)
    {
        std::vector<int> indexList = faces[index];
        
        //auto face = _m.add_face(_m.vertex_handle(indexList[0]), _m.vertex_handle(indexList[1]),
        //                        _m.vertex_handle(indexList[2]), _m.vertex_handle(indexList[3]));
        
        // Randy replaced above line with below line
        auto face = _m.addPolygonFace({_m.idToVert[indexList[0]], _m.idToVert[indexList[1]],  _m.idToVert[indexList[2]], _m.idToVert[indexList[3]]});



        // Randy replaced above line with below two lines

        printf("f%d: ", index);
        for (int id : indexList)
        {
            printf("%d ", id); // OBJ uses 1-based arrays...
        }
        printf("\n");
    }

    return true;
}

bool CMeshMerger::subdivide(CMeshImpl& _m, unsigned int n, bool isSharp)
{
    // Project AddOffset - reimplement this later
    return true;
}

void CMeshMerger::split_face(CMeshImpl& _m, const CMeshImpl::FaceHandle& _fh)
{
    // Project AddOffset - reimplement this later
}

void CMeshMerger::split_edge(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh)
{
    // Project AddOffset - reimplement this later
}

void CMeshMerger::compute_midpoint(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh,
                                   const bool _update_points)
{
    // Project AddOffset - reimplement this later
}

void CMeshMerger::update_vertex(CMeshImpl& _m, const CMeshImpl::VertexHandle& _vh)
{ // Project AddOffset - reimplement this later
}

}