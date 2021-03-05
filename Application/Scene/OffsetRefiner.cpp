// Project AddOffset - Zachary's changes. I modified this file extensively to work with the new winged edge DS
#include "OffsetRefiner.h"
#undef M_PI

namespace Nome::Scene
{

COffsetRefiner::COffsetRefiner(DSMesh& _m, bool offsetFlag)
{
    this->currMesh;
    currMesh = _m;
    flag = offsetFlag;

    size_t numFaces = currMesh.faceList.size(); // Mesh.n_faces();
    faceVertices.resize(numFaces);
    newFaceVertices.resize(numFaces);
    for (auto face : currMesh.faceList)
    {
        int i = 0;
        for (auto vertex : face->vertices)
            i++;
        faceVertices[face->id] = i;
    }
    //Mesh.request_face_normals(); 

    size_t numVertices = currMesh.vertList.size(); // Mesh.n_vertices();
    vertexEdges.resize(numVertices);
    newVertices.resize(numVertices);
    //for (auto vertex : currMesh.vertList)//Mesh.vertices())
    //{
    //    int i = 0;
    //    for (auto edge : vertex.edges())
    //        i++;
    //    vertexEdges[vertex.idx()] = i; // Randy note: so seems like i is the # of edges that reference the vert
    //}

    // Randy added this. Should hopefully initialize vertexEdges the same way as before
    for (auto& Pair : currMesh.edgeTable) {
        Vertex* currVert = Pair.first;
        std::vector<Edge*> vertEdges = Pair.second;
        vertexEdges[currVert->ID] = vertEdges.size();
    }
}

void COffsetRefiner::Refine(float height, float width)
{
    bool needGrid = (width > 0);
    bool needOffset = (height > 0);
    for (auto vertex : currMesh.vertList)
    {
        if (vertexEdges[vertex->ID] < 2)
        {
            continue;
        }
        generateNewVertices(vertex, height);
    }

    auto faces = currMesh.faceList; // Mesh.faces().to_vector();
    for (auto face : faces)
    {
        if (needGrid)
        {
            generateNewFaceVertices(face, width, height);
        }
        generateNewFaces(face, needGrid, needOffset);
    }

    if (needOffset)
    {
        for (auto face : faces)
            closeFace(face);
    }
}

void COffsetRefiner::generateNewVertices(Vertex * vertex, float height)
{
    Vector3 point = vertex->position;  // getPosition(vertex);

    if (height <= 0)
    {
        addPoint(point);
        newVertices[vertex->ID] = OffsetVerticesInfo { point, (int)offsetVertices.size() - 1 };
        return;
    }

    Vector3 sumEdges;

    if (vertexEdges[vertex->ID] > 2)
    {
        for (auto edge : currMesh.edgeTable[vertex])//vertex.edges())
        {
            sumEdges += getEdgeVector(edge, vertex).Normalized();
        }
    }
    else
    {
        std::vector<Edge*> edges = currMesh.edgeTable[vertex]; // vertex.edges().to_vector();
        Vector3 edge1 = getEdgeVector(edges[0], vertex);
        Vector3 edge2 = getEdgeVector(edges[1], vertex);

        sumEdges = crossProduct(edge1, edge2);
    }

    sumEdges.Normalize();
    sumEdges *= height / 2;

    Vector3 newPoint1 = point - sumEdges;
    Vector3 newPoint2 = point + sumEdges;

    addPoint(newPoint1);
    addPoint(newPoint2);

    int size = offsetVertices.size();
    newVertices[vertex->ID] = OffsetVerticesInfo { newPoint1, size - 2, newPoint2, size - 1 };
}

void COffsetRefiner::generateNewFaceVertices(Face* face, float width, float height)
{
    size_t length = faceVertices[face->id];

    std::vector<Vector3> facePoints;
    std::vector<int> idxList;
    for (auto vertex : face->vertices)
    {
        facePoints.push_back(vertex->position);  // getPosition(vertex));
        idxList.push_back(vertex->ID);
    }

    for (size_t index = 0; index < length; index++)
    {
        Vector3 curPoint = facePoints[index];
        Vector3 prevPoint = facePoints[(index - 1 + length) % length];
        Vector3 nextPoint = facePoints[(index + 1) % length];

        Vector3 prevPath = prevPoint - curPoint;
        Vector3 curPath = nextPoint - curPoint;

        if (flag)
        {
            prevPath.Normalize();
            curPath.Normalize();
        }

        float angle = getAngle(prevPath, curPath);
        Vector3 offsetVector = (prevPath + curPath).Normalized() * width / sinf(angle / 2);

        Vector3 norm = crossProduct(prevPath, curPath).Normalized() * height / 2;

        Vector3 newPoint1 = curPoint + offsetVector - norm;
        Vector3 newPoint2 = curPoint + offsetVector + norm;

        addPoint(newPoint1);
        addPoint(newPoint2);

        int size = offsetVertices.size();
        newFaceVertices[face->id][idxList[index]] =
            OffsetVerticesInfo { newPoint1, size - 2, newPoint2, size - 1 };
    }
}

void COffsetRefiner::generateNewFaces(Face * face, bool needGrid,
                                      bool needOffset)
{

    if (!needGrid)
    {
        std::vector<int> indexList1, indexList2;
        std::vector<Vertex *> vertices1, vertices2;
        for (auto vertex : face->vertices)
        {
            int index = vertex->ID;

            int topIndex = newVertices[index].topIndex;
            int bottomIndex = newVertices[index].bottomIndex;
            indexList1.push_back(topIndex);
            indexList2.push_back(bottomIndex);
            vertices1.push_back(newVertexHandleList[topIndex]);
            vertices2.push_back(newVertexHandleList[bottomIndex]);
        }

        std::reverse(indexList2.begin(), indexList2.end());
        std::reverse(vertices2.begin(), vertices2.end());

        offsetFaces.push_back(indexList1);
        offsetFaces.push_back(indexList2);

        currMesh.addFace(vertices1);
        currMesh.addFace(vertices2);

        return; 
    }

    int faceId = face->id;
    std::vector<int> idxList;

    for (auto vertex : face->vertices)
        idxList.push_back(vertex->ID);

    for (size_t i = 0; i < idxList.size(); i++)
    {
        int vertex1Id = idxList[i];
        int vertex2Id = idxList[(i + 1) % idxList.size()];

        int vertex1TopIndex = newVertices[vertex1Id].topIndex;
        int vertex1TopInsideIndex = newFaceVertices[faceId][vertex1Id].topIndex;

        int vertex2TopIndex = newVertices[vertex2Id].topIndex;
        int vertex2TopInsideIndex = newFaceVertices[faceId][vertex2Id].topIndex;

        std::vector<int> faceIndexList;
        faceIndexList = {
            vertex1TopInsideIndex,
            vertex1TopIndex,
            vertex2TopIndex,
            vertex2TopInsideIndex,
        };
        offsetFaces.push_back(faceIndexList);

        currMesh.addFace(
            { newVertexHandleList[vertex1TopInsideIndex], newVertexHandleList[vertex1TopIndex],
              newVertexHandleList[vertex2TopIndex], newVertexHandleList[vertex2TopInsideIndex] });

        //Mesh.add_face(newVertexHandleList[vertex1TopInsideIndex],
        //              newVertexHandleList[vertex1TopIndex], newVertexHandleList[vertex2TopIndex],
        //              newVertexHandleList[vertex2TopInsideIndex]);

        if (needOffset)
        {
            int vertex1BottomIndex = newVertices[vertex1Id].bottomIndex;
            int vertex1BottomInsideIndex = newFaceVertices[faceId][vertex1Id].bottomIndex;
            int vertex2BottomIndex = newVertices[vertex2Id].bottomIndex;
            int vertex2BottomInsideIndex = newFaceVertices[faceId][vertex2Id].bottomIndex;

            faceIndexList = { vertex1BottomInsideIndex, vertex1TopInsideIndex,
                              vertex2TopInsideIndex, vertex2BottomInsideIndex };
            offsetFaces.push_back(faceIndexList);

            currMesh.addFace({ newVertexHandleList[vertex1BottomInsideIndex],
                                      newVertexHandleList[vertex1TopInsideIndex],
                                      newVertexHandleList[vertex2TopInsideIndex],
                                      newVertexHandleList[vertex2BottomInsideIndex] });
   /*         Mesh.add_face(newVertexHandleList[vertex1BottomInsideIndex],
                          newVertexHandleList[vertex1TopInsideIndex],
                          newVertexHandleList[vertex2TopInsideIndex],
                          newVertexHandleList[vertex2BottomInsideIndex]);*/

            faceIndexList = { vertex1BottomIndex, vertex1BottomInsideIndex,
                              vertex2BottomInsideIndex, vertex2BottomIndex };
            offsetFaces.push_back(faceIndexList);
            currMesh.addFace({ newVertexHandleList[vertex1BottomIndex],
                                newVertexHandleList[vertex1BottomInsideIndex],
                                newVertexHandleList[vertex2BottomInsideIndex],
                                newVertexHandleList[vertex2BottomInsideIndex] });
           /* Mesh.add_face(newVertexHandleList[vertex1BottomIndex],
                          newVertexHandleList[vertex1BottomInsideIndex],
                          newVertexHandleList[vertex2BottomInsideIndex],
                          newVertexHandleList[vertex2BottomInsideIndex]);*/
        }
    }
}

void COffsetRefiner::closeFace(Face* face)
{
    std::vector<int> idxList;
    for (auto vertex : face->vertices)
        idxList.push_back(vertex->ID);

    for (size_t i = 0; i < idxList.size(); i++)
    {
        int vertex1Id = idxList[i];
        int vertex2Id = idxList[(i + 1) % idxList.size()];

        int vertex1TopIndex = newVertices[vertex1Id].topIndex;
        int vertex1BottomIndex = newVertices[vertex1Id].bottomIndex;
        int vertex2TopIndex = newVertices[vertex2Id].topIndex;
        int vertex2BottomIndex = newVertices[vertex2Id].bottomIndex;

        Vertex * vertex1Top = newVertexHandleList[vertex1TopIndex];
        Vertex *  vertex1Bottom = newVertexHandleList[vertex1BottomIndex];
        Vertex * vertex2Top = newVertexHandleList[vertex2TopIndex];
        Vertex * vertex2Bottom = newVertexHandleList[vertex2BottomIndex];

        vector<Edge*> boundaryEdgeList1 = currMesh.boundaryEdgeList(); // Randy added this. needed to check if vert is in boundary

        // check to see if the vertex is on an edge that is on a boundary.. In openMesh, is_boundary() checked if a vertex is adjacent to a boundary edge
        



        // Randy added the next few lines. allAdjacentEdges contains the adjacent edges for the relevant verts
        auto vert1TopETable = currMesh.edgeTable[vertex1Top];
        std::vector<Edge*> allAdjacentEdges = vert1TopETable; 
        auto vert1BotETable = currMesh.edgeTable[vertex1Bottom];
        auto vert2TopETable = currMesh.edgeTable[vertex2Top];
        auto vert2BotETable = currMesh.edgeTable[vertex2Bottom];
        allAdjacentEdges.insert(allAdjacentEdges.end(), vert1BotETable.begin(), vert1BotETable.end());
        allAdjacentEdges.insert(allAdjacentEdges.end(), vert2TopETable.begin(), vert2TopETable.end());
        allAdjacentEdges.insert(allAdjacentEdges.end(), vert2BotETable.begin(), vert2BotETable.end());

        bool onBoundary = true;
        for (Edge * adjEdge : allAdjacentEdges) {
            std::vector<Edge*> boundaryList = currMesh.boundaryEdgeList();
            if (std::find(boundaryList.begin(), boundaryList.end(), adjEdge) == boundaryList.end()) { // if none of the relevant verts are on a boundary edge
                // if one of 
                onBoundary = false;
            }
        }
        if (onBoundary) {
            currMesh.addFace({ vertex1Top, vertex1Bottom, vertex2Bottom, vertex2Top });
        
            std::vector<int> faceIndexList = { vertex1TopIndex, vertex1BottomIndex,
                                                vertex2BottomIndex, vertex2TopIndex };
            offsetFaces.push_back(faceIndexList);
        }
        //if (vertex1Top.is_boundary() && vertex1Bottom.is_boundary() && vertex2Top.is_boundary()
        //    && vertex2Bottom.is_boundary())
        //{
        //    Mesh.add_face(vertex1Top, vertex1Bottom, vertex2Bottom, vertex2Top);
        //    std::vector<int> faceIndexList = { vertex1TopIndex, vertex1BottomIndex,
        //                                       vertex2BottomIndex, vertex2TopIndex };
        //    offsetFaces.push_back(faceIndexList);
        //}
    }
}

//Vector3 COffsetRefiner::getPosition(OpenMesh::SmartVertexHandle vertex)
//{
//    const auto& pos = Mesh.point(vertex);
//    return Vector3(pos[0], pos[1], pos[2]);
//}

Vector3 COffsetRefiner::getEdgeVector(Edge * edge,
                                      Vertex * vertex)
{
    Vector3 tempPoint;
    if (edge->va == vertex) //if (edge.v0() == vertex)
    {
        tempPoint = edge->vb->position; // getPosition(edge.v1());
    }
    else
    {
        tempPoint = edge->va->position;  // getPosition(edge.v0());
    }

    return tempPoint - vertex->position;  // getPosition(vertex);
}

Vector3 COffsetRefiner::crossProduct(Vector3 vectorA, Vector3 vectorB)
{
    return Vector3(vectorA.y * vectorB.z - vectorA.z * vectorB.y,
                   vectorA.z * vectorB.x - vectorA.x * vectorB.z,
                   vectorA.x * vectorB.y - vectorA.y * vectorB.x);
}

float COffsetRefiner::getAngle(Vector3 vectorA, Vector3 vectorB)
{
    float value = vectorA.DotProduct(vectorB) / vectorA.Length() / vectorB.Length();
    float epsilon = 1e-4;
    if (fabs(value - 1) < epsilon)
    {
        return 0;
    }
    if (fabs(value + 1) < epsilon)
    {
        return (float)tc::M_PI;
    }
    if (fabs(value) < epsilon)
    {
        return (float)tc::M_PI / 2;
    }

    return acosf(vectorA.DotProduct(vectorB) / vectorA.Length() / vectorB.Length());
}

void COffsetRefiner::addPoint(Vector3 vector)
{
    //newVertexHandleList.push_back(Mesh.add_vertex(CMeshImpl::Point(vector.x, vector.y, vector.z)));
   
    // Randy replaced above with below two lines
    Vertex* newVert = new Vertex(vector.x, vector.y, vector.z, currMesh.vertList.size());
    currMesh.addVertex(newVert);
    newVertexHandleList.push_back(newVert);
    offsetVertices.push_back(vector);
}

}