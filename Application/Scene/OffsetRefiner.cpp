// Project AddOffset - Zachary's changes. I modified this file extensively to work with the new
// winged edge DS
#include "OffsetRefiner.h"
#undef M_PI

namespace Nome::Scene
{

COffsetRefiner::COffsetRefiner(DSMesh& _m, bool offsetFlag)
{
    currMesh = _m.newMakeCopy(); // Project AddOffset

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
    // Mesh.request_face_normals();

    size_t numVertices = currMesh.vertList.size(); // Mesh.n_vertices();
    vertexEdges.resize(numVertices);
    newVertices.resize(numVertices);
    // for (auto vertex : currMesh.vertList)//Mesh.vertices())
    //{
    //    int i = 0;
    //    for (auto edge : vertex.edges())
    //        i++;
    //    vertexEdges[vertex.idx()] = i; // Randy note: so seems like i is the # of edges that
    //    reference the vert
    //}

    for (auto& Pair : currMesh.randyedgeTable)
    {
        Vertex* currVert = Pair.first;
        std::vector<Edge*> vertEdges = Pair.second;
        vertexEdges[currVert->ID] = vertEdges.size();
    }
}

void COffsetRefiner::Refine(float height, float width)
{
    bool needGrid = (width > 0);
    bool needOffset = (height > 0);

    auto vertSize = currMesh.vertList.size();

    for (int i = 0; i < vertSize; i++) // Randy perhaps vertList is changing size wrong
    {
        auto vertex = currMesh.vertList[i];
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

    currMesh.buildBoundary(); // Randy added these on 2/26
    currMesh.computeNormals();
}


void COffsetRefiner::generateNewVertices(Vertex* vertex, float height)
{
    Vector3 point = vertex->position; // getPosition(vertex);

    if (height <= 0)
    {
        addPoint(point);
        // newVertices[vertex->ID] = OffsetVerticesInfo { point, (int)offsetVertices.size() - 1 };
        newVertices[vertex->ID] =
            OffsetVerticesInfo { vertex, (int)offsetVertices.size() - 1 }; // Randy replaced above
        return;
    }

    Vector3 sumEdges;

    if (vertexEdges[vertex->ID] > 2)
    {
        std::cout << "vert edges is > 2" << std::endl;
        auto edges = currMesh.randyedgeTable[vertex];
        for (auto edge : edges)
        {
            sumEdges += getEdgeVector(edge, vertex).Normalized();
        }
    }
    else
    {
        std::cout << "vert edges is <= 2" << std::endl;
        std::vector<Edge*> edges = currMesh.randyedgeTable[vertex]; // vertex.edges().to_vector();

        Vector3 edge1;
        Vector3 edge2;
        bool forwardDir = (edges[1]->edge_ID - edges[0]->edge_ID) == 1;
        if (forwardDir)
        {
            std::cout << "1st edge of vert: " << edges[0]->edge_ID << std::endl;
            std::cout << "2nd edge of this vert: " << edges[1]->edge_ID << std::endl;
            edge1 = getEdgeVector(edges[0], vertex);
            edge2 = getEdgeVector(edges[1], vertex);
        }
        else // beginning vertex. need to reverse order to get same cross product direction
        {
            std::cout << "1st edge of vert: " << edges[1]->edge_ID << std::endl;
            std::cout << "2nd edge of this vert: " << edges[0]->edge_ID << std::endl;
            edge1 = getEdgeVector(edges[1], vertex);
            edge2 = getEdgeVector(edges[0], vertex);
        }

        sumEdges = crossProduct(edge1, edge2);
    }
    sumEdges.Normalize();
    sumEdges *= height / 2;

    Vector3 newPoint1 = point - sumEdges;
    Vector3 newPoint2 = point + sumEdges;
    Vertex* newVert1 = addPoint(newPoint1);
    Vertex* newVert2 = addPoint(newPoint2);
    int size = offsetVertices.size();
    newVertices[vertex->ID] = OffsetVerticesInfo { newVert1, size - 2, newVert2, size - 1 };
}


void COffsetRefiner::generateNewFaceVertices(Face* face, float width, float height)
{
    size_t length = faceVertices[face->id];

    std::vector<Vector3> facePoints;
    std::vector<int> idxList;
    for (auto vertex : face->vertices)
    {
        facePoints.push_back(vertex->position); // getPosition(vertex));
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

        Vertex* newVert1 = addPoint(newPoint1);
        Vertex* newVert2 = addPoint(newPoint2);
        int size = offsetVertices.size();
        newFaceVertices[face->id][idxList[index]] =
            OffsetVerticesInfo { newVert1, size - 2, newVert2,
                                 size - 1 }; // Randy changed this to be new Vert
    }
}

void COffsetRefiner::generateNewFaces(Face* face, bool needGrid, bool needOffset)
{

    if (!needGrid)
    {
        std::vector<int> indexList1, indexList2;
        std::vector<Vertex*> vertices1, vertices2;
        for (auto vertex : face->vertices)
        {
            int index = vertex->ID;

            int topIndex = newVertices[index].topIndex;
            int bottomIndex = newVertices[index].bottomIndex;
            indexList1.push_back(topIndex);
            indexList2.push_back(bottomIndex);
            vertices1.push_back(newVertexList[topIndex]);
            vertices2.push_back(newVertexList[bottomIndex]);
        }

        std::reverse(indexList2.begin(), indexList2.end());
        std::reverse(vertices2.begin(), vertices2.end());

        // offsetFaces.push_back(indexList1);
        // offsetFaces.push_back(indexList2);

        // Randy changed above line to the following
        Face* offsetFace1 = new Face(vertices1); // takes in a vector of Vertex objects
        Face* offsetFace2 = new Face(vertices2); // takes in a vector of Vertex objects
        offsetFaces.push_back(offsetFace1);
        offsetFaces.push_back(offsetFace2);

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

        // Randy added below lines to hopefully replace above lines
        Vertex* vertex1Top = newVertices[vertex1Id].topVert;
        Vertex* vertex1TopInside = newFaceVertices[faceId][vertex1Id].topVert;
        Vertex* vertex2Top = newVertices[vertex2Id].topVert;
        Vertex* vertex2TopInside = newFaceVertices[faceId][vertex2Id].topVert;

        // std::vector<int> faceIndexList;
        // faceIndexList = {
        //    vertex1TopInsideIndex,
        //    vertex1TopIndex, // Randy the bug is here, for some reason always 0
        //    vertex2TopIndex, // Randy the bug is here, for some reason always 0
        //    vertex2TopInsideIndex,
        //};
        // offsetFaces.push_back(faceIndexList);

        // Randy below line to hopefully replace above lines
        Face* offsetFaceTop =
            currMesh.addFace({ vertex1TopInside, vertex1Top, vertex2Top, vertex2TopInside });
        offsetFaces.push_back(offsetFaceTop);

        // Mesh.add_face(newVertexList[vertex1TopInsideIndex],
        //              newVertexList[vertex1TopIndex], newVertexList[vertex2TopIndex],
        //              newVertexList[vertex2TopInsideIndex]);

        if (needOffset)
        {
            Vertex* vertex1Bottom = newVertices[vertex1Id].bottomVert;
            Vertex* vertex1BottomInside = newFaceVertices[faceId][vertex1Id].bottomVert;
            Vertex* vertex2Bottom = newVertices[vertex2Id].bottomVert;
            Vertex* vertex2BottomInside = newFaceVertices[faceId][vertex2Id].bottomVert;

            Face* offsetFaceBotTop = currMesh.addFace(
                { vertex1BottomInside, vertex1TopInside, vertex2TopInside, vertex2BottomInside });
            offsetFaces.push_back(offsetFaceBotTop);

            Face* offsetFaceBot =
                currMesh.addFace({ vertex1Bottom, vertex1BottomInside, vertex2BottomInside,
                                   vertex2Bottom }); // TODO // randy check if the two BottomInside
                                                     // is a typo or should i switch to it
            offsetFaces.push_back(offsetFaceBot);
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

        Vertex* vertex1Top = newVertexList[vertex1TopIndex];
        Vertex* vertex1Bottom = newVertexList[vertex1BottomIndex];
        Vertex* vertex2Top = newVertexList[vertex2TopIndex];
        Vertex* vertex2Bottom = newVertexList[vertex2BottomIndex];

        std::vector<Edge*> boundaryEdgeList1 =
            currMesh.boundaryEdgeList(); // Randy added this. needed to check if vert is in boundary

        // check to see if the vertex is on an edge that is on a boundary.. In openMesh,
        // is_boundary() checked if a vertex is adjacent to a boundary edge

        std::vector<Vertex*> verts = { vertex1Top, vertex1Bottom, vertex2Top, vertex2Bottom };
        std::vector<Edge*> boundaryList = currMesh.boundaryEdgeList();
        bool allOnBoundary = true;
        for (auto vert : verts)
        {
            bool onBoundary = false;
            std::vector<Edge*> boundaryList = currMesh.boundaryEdgeList();
            std::vector<Edge*> adjEdges = currMesh.randyedgeTable[vertex1Top];
            for (auto adjEdge : adjEdges)
            {
                if (std::find(boundaryList.begin(), boundaryList.end(), adjEdge)
                    != boundaryList.end())
                {
                    // just need one of the vert's edges to be onboundary
                    onBoundary = true;
                }
            }
            allOnBoundary = allOnBoundary && onBoundary;
        }

        if (allOnBoundary)
        {
            // currMesh.addPolygonFace({ vertex1Top, vertex1Bottom, vertex2Bottom, vertex2Top });

            // std::vector<int> faceIndexList = { vertex1TopIndex, vertex1BottomIndex,
            //                                    vertex2BottomIndex, vertex2TopIndex };
            // offsetFaces.push_back(faceIndexList);

            // Randy added below to replace above lines
            Face* offsetFaceClose =
                currMesh.addFace({ vertex1Top, vertex1Bottom, vertex2Bottom, vertex2Top });
            offsetFaces.push_back(offsetFaceClose);
        }
        // if (vertex1Top.is_boundary() && vertex1Bottom.is_boundary() && vertex2Top.is_boundary()
        //    && vertex2Bottom.is_boundary())
        //{
        //    Mesh.add_face(vertex1Top, vertex1Bottom, vertex2Bottom, vertex2Top);
        //    std::vector<int> faceIndexList = { vertex1TopIndex, vertex1BottomIndex,
        //                                       vertex2BottomIndex, vertex2TopIndex };
        //    offsetFaces.push_back(faceIndexList);
        //}
    }
}

// Vector3 COffsetRefiner::getPosition(OpenMesh::SmartVertexHandle vertex)
//{
//    const auto& pos = Mesh.point(vertex);
//    return Vector3(pos[0], pos[1], pos[2]);
//}

Vector3 COffsetRefiner::getEdgeVector(Edge* edge, Vertex* vertex)
{
    Vector3 tempPoint;
    if (edge->va == vertex) // if (edge.v0() == vertex)
    {
        tempPoint = edge->vb->position; // getPosition(edge.v1());
    }
    else
    {
        tempPoint = edge->va->position; // getPosition(edge.v0());
    }

    return tempPoint - vertex->position; // getPosition(vertex);
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

Vertex* COffsetRefiner::addPoint(Vector3 vector)
{
    // newVertexList.push_back(Mesh.add_vertex(CMeshImpl::Point(vector.x, vector.y, vector.z)));

    // Randy replaced above with below two lines
    Vertex* newVert = new Vertex(vector.x, vector.y, vector.z, currMesh.vertList.size());
    newVert->name = "offsetRefiner" + std::to_string(currMesh.vertList.size()) + "vert";
    currMesh.addVertex(newVert);
    newVertexList.push_back(newVert);
    // offsetVertices.push_back(vector);

    offsetVertices.push_back(newVert); // Randy replaced above with this
    return newVert;
}

}