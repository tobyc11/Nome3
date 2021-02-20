#pragma once
#include "Mesh.h"
#include <LangUtils.h>
#include <OpenMesh/Tools/Subdivider/Uniform/CatmullClarkT.hh>
namespace Nome::Scene
{

class CMeshMerger : public CMesh
{
public:
    using Super = CMesh;
    CMeshMerger() = default;
    CMeshMerger(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;

    // No update yet, please just use one time
    void MergeIn(const CMeshInstance& meshInstance);

    // Changed during Project SwitchDS
    void Catmull();

    // hacky way to fix Mobius face, where CCW goes to CW edge orientation
    std::vector<Vector3> fixEdgeOrientation(std::vector<Vector3> facePositions);

    
    
    //below are new

    // Constructor.
    //Subdivision();

    // Constructor.
    // @param mesh: the reference of given mesh.
    //Subdivision(Mesh mesh);

    // The integration of subdivision.
    // @param level: level of Catmull-Clark subdivision.
    void ccSubdivision(int level); // Randy changed from DSMesh to void

private:
    std::pair<Vertex*, float> FindClosestVertex(const tc::Vector3& pos);

    unsigned int VertCount = 0;
    unsigned int FaceCount = 0;

    //below methods are new
    // Construct face points in Catmull-Clark subdivision.
    // Computed values are stored in face.facepoint. Add new Vertices to vertTable.
    // @param newVertList: The list of vertices for next level mesh.
    void makeFacePoints(vector<Vertex*>& newVertList);
    // Construct edge points in Catmull-Clark subdivision.
    // Computed values are stored in edge.edgepoint. Add new Vertices to vertTable.
    // @param newVertList: The list of vertices for next level mesh.
    void makeEdgePoints(vector<Vertex*>& newVertList);
    // Construct vertex points in Catmull-Clark subdivision with DeRose et al's Equation.
    // By modifying the original position and pointers of the vertex.
    // @param newVertList: The list of vertices for next level mesh.
    void makeVertexPointsD(vector<Vertex*>& newVertList);
    // Similar to the above function, but with Catmull-Clark's equation.
    // @param newVertList: The list of vertices for next level mesh.
    void makeVertexPointsC(vector<Vertex*>& newVertList);
    // Construct a new mesh after given new facepoints, edgepoints, and vertexpoints.
    // @param newFaceList: the list of faces for the next level mesh.
    void compileNewMesh(vector<Face*>& newFaceList);
    // In order to redo the subdivision, we need to set all next level subdivision to null
    void setAllNewPointPointersToNull();
};

}