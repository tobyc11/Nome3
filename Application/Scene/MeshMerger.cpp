#include "MeshMerger.h"
#include "Subdivision.h"


#include <unordered_map>

namespace Nome::Scene
{
DEFINE_META_OBJECT(CMeshMerger)
{
    BindNamedArgument(&CMeshMerger::Level, "sd_level", 0);
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
    if (subdivisionLevel == 0 || currMesh.isEmpty()) {
        return;
    }
    subdivide(currMesh, subdivisionLevel);
    cout << "done with subdiv" << endl;
    currMesh.buildBoundary();
    currMesh.computeNormals();
    //above is new stuff   
   
}

void CMeshMerger::MergeIn(CMeshInstance& meshInstance)
{
    auto tf = meshInstance.GetSceneTreeNode()->L2WTransform.GetValue(
        tc::Matrix3x4::IDENTITY); // The transformation matrix is the identity matrix by default
    auto& otherMesh = meshInstance.GetDSMesh(); // Getting OpeshMesh implementation of a mesh. This allows us to
                                    // traverse the mesh's vertices/faces

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
        return; // skip for now, dont merge polyline entities
    }

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

            auto* copiedVert = new Vertex(worldPos.x, worldPos.y, worldPos.z, currMesh.nameToVert.size());
            copiedVert->name = "copiedVert" + std::to_string(currMesh.nameToVert.size()); // Randy this was causing the bug!!!!!!! the name was the same. so nameToVert remained size == 1
            MergedMesh.addVertex(copiedVert);
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
            verts.emplace_back(vertMap[vert]);
        } // Add the vertex handles
        //auto fnew =
        //Mesh.add_face(verts); // add_face processes the merger vertex handles and adds the face
                                  // into the merger mesh (Mesh refers to the merger mesh here)
        Face* copiedFace = new Face(verts);
        MergedMesh.addFace(verts);
        std::string fName = "v" + std::to_string(FaceCount);
        FaceCount++;
    }

    MergedMesh.buildBoundary();
    MergedMesh.computeNormals();
    currMesh.clear();
    currMesh = MergedMesh;

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


void CMeshMerger::MergeClear() {
    currMesh.clear();
    MergedMesh.clear();
}




bool CMeshMerger::subdivide(DSMesh& _m, unsigned int n) const
{

    // Instantiate a Far::TopologyRefiner from the descriptor
    Far::TopologyRefiner * refiner = GetRefiner(_m, isSharp);


    refiner->RefineUniform(Far::TopologyRefiner::UniformOptions(n));

    std::vector<Vertex> vbuffer(refiner->GetNumVerticesTotal());
    Vertex * verts = &vbuffer[0];

    for (auto v_itr : _m.vertList) {
        verts[v_itr->ID].SetPosition(v_itr->position.x, v_itr->position.y, v_itr->position.z);
    }



    // Interpolate vertex primvar data
    Far::PrimvarRefiner primvarRefiner(*refiner);

    Vertex * src = verts;
    for (int level = 1; level <= n; ++level) {
        Vertex * dst = src + refiner->GetLevel(level-1).GetNumVertices();
        primvarRefiner.Interpolate(level, src, dst);
        src = dst;
    }
    _m.clear();
    { // Output OBJ of the highest level refined -----------
        /// to debug
        Far::TopologyLevel const & refLastLevel = refiner->GetLevel(n);
        for (int i = 0; i <= n; ++i)
        {
            printf("level:%d has %d vertices\n", i, refiner->GetLevel(i).GetNumVertices());
        }
        printf("total number of %d vertices\n", refiner->GetNumVerticesTotal());

        int nverts = refLastLevel.GetNumVertices();
        int nfaces = refLastLevel.GetNumFaces();

        // Print vertex positions
        int firstOfLastVerts = refiner->GetNumVerticesTotal() - nverts;


        printf("============ output vertices======\n");
        for (int vert = 0; vert < nverts; ++vert) {
            float const * pos = verts[vert + firstOfLastVerts].GetPosition();
            _m.addVertex(pos[0], pos[1], pos[2]);
            printf("v %f %f %f\n", pos[0], pos[1], pos[2]);
        }

        // Print faces
        printf("============ output faces======\n");
        for (int face = 0; face < nfaces; ++face) {
            Far::ConstIndexArray fverts = refLastLevel.GetFaceVertices(face);

            // all refined Catmark faces should be quads
            assert(fverts.size()==4);
            vector<Vertex*> vertices;
            for (int i = 0; i < 4; ++i)
            {
                vertices.push_back(_m.vertList.at(fverts[i]));
            }
            _m.addFace(vertices);

            printf("f ");
            for (int vert=0; vert<fverts.size(); ++vert) {
                printf("%d ", fverts[vert]+1); // OBJ uses 1-based arrays...
            }
            printf("\n");


        }
    }


    return true;
}


}