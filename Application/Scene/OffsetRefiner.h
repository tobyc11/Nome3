// Project AddOffset - Zachary's changes
#pragma once
#include "Mesh.h"
#include <unordered_map>

namespace Nome::Scene
{

class COffsetRefiner
{
public:
    COffsetRefiner(DSMesh& _m, bool offsetFlag);
    void Refine(float height, float width);
    std::vector<Vector3> GetVertices() { return offsetVertices; };
    std::vector<std::vector<int>> GetFaces() { return offsetFaces; };

private:
    struct OffsetVerticesInfo
    {
        Vector3 topPos;
        int topIndex;
        Vector3 bottomPos;
        int bottomIndex;
    };

    void generateNewVertices(Vertex * vertex, float height);
    void generateNewFaceVertices(Face * face, float width, float height);
    void generateNewFaces(Face * face, bool needGrid, bool needOffset);
    void closeFace(Face * face);

    // Vector3 getPosition(Vertex * vertex); Randy commented this out. With the new DataStructure, we can access position directly from Vertex
    Vector3 getEdgeVector(Edge * edge, Vertex * vertex);

    float getAngle(Vector3 vectorA, Vector3 vectorB);
    Vector3 crossProduct(Vector3 vectorA, Vector3 vectorB);

    std::vector<int> faceVertices;
    std::vector<int> vertexEdges;
    std::vector<std::unordered_map<int, OffsetVerticesInfo>> newFaceVertices;
    std::vector<OffsetVerticesInfo> newVertices;
    //CMeshImpl Mesh; Project AddOffset
    DSMesh currMesh; // Randy added this
    bool flag;

    std::vector<Vector3> offsetVertices;
    std::vector<std::vector<int>> offsetFaces;

    std::vector<Vertex *> newVertexHandleList;
    void addPoint(Vector3 vector);
};

}