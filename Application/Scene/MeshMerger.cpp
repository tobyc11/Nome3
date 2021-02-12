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

void CMeshMerger::Catmull()
{
    ccSubdivision(2);
    cout << "done with subdiv" << endl;
    currMesh.buildBoundary();
    currMesh.computeNormals();
    //above is new stuff   
   
}

void CMeshMerger::MergeIn(const CMeshInstance& meshInstance)
{
    auto tf = meshInstance.GetSceneTreeNode()->L2WTransform.GetValue(
        tc::Matrix3x4::IDENTITY); // The transformation matrix is the identity matrix by default
    auto& otherMesh = meshInstance.GetDSMesh(); // Getting OpeshMesh implementation of a mesh. This allows us to
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



//CMeshMerger::Subdivision(Mesh mesh) { currMesh = mesh; }

void CMeshMerger::makeFacePoints(vector<Vertex*>& newVertList)
{
    vector<Face*>::iterator fIt;
    for (fIt = currMesh.faceList.begin(); fIt < currMesh.faceList.end(); fIt++)
    {
        Vertex* newFacePoint = new Vertex;
        Vector3 newFacePointPosition = Vector3(0, 0, 0);
        Face* currFace = (*fIt);
        Edge* firstEdge = currFace->oneEdge;
        if (firstEdge == NULL)
        {
            cout << "ERROR: This face (with ID) does not have a sideEdge." << endl;
            exit(1);
        }
        Edge* currEdge = firstEdge;
        uint counter = 0;
        Vertex* currVert;
        do
        {
            if (currFace == currEdge->fa)
            {
                currVert = currEdge->vb;
                currEdge = currEdge->nextVbFa;
            }
            else if (currFace == currEdge->fb)
            {
                if (currEdge->mobius)
                {
                    currVert = currEdge->vb;
                    currEdge = currEdge->nextVbFb;
                }
                else
                {
                    currVert = currEdge->va;
                    currEdge = currEdge->nextVaFb;
                }
            }
            newFacePointPosition += currVert->position;
            counter += 1;
        } while (currEdge != firstEdge);
        newFacePointPosition /= counter;
        newFacePoint->position = newFacePointPosition;
        newFacePoint->ID = newVertList.size();
        // cout<<"New Face Point: ID: "<< newFacePoint -> ID <<" Position: "<< (newFacePoint ->
        // position)[0]<<" "<<(newFacePoint -> position)[1]<<" "<<(newFacePoint ->
        // position)[2]<<endl;
        currFace->facePoint = newFacePoint;
        newVertList.push_back(newFacePoint);
    }
}

void CMeshMerger::makeEdgePoints(vector<Vertex*>& newVertList)
{
    vector<Face*>::iterator fIt;
    for (fIt = currMesh.faceList.begin(); fIt < currMesh.faceList.end(); fIt++)
    {
        Face* currFace = (*fIt);
        Edge* firstEdge = currFace->oneEdge;
        Edge* currEdge = firstEdge;
        Vertex* currVert;
        do
        {
            Vertex* newEdgePoint = new Vertex;
            if (currEdge->edgePoint == NULL)
            {
                if (currEdge->isSharp)
                {
                    newEdgePoint->position =
                        (currEdge->va->position + currEdge->vb->position) / (float)2.0;
                }
                else
                {
                    Vertex* faceVert1 = currEdge->fa->facePoint;
                    Vertex* edgeVert1 = currEdge->va;
                    Vertex* edgeVert2 = currEdge->vb;
                    Vertex* faceVert2 = currEdge->fb->facePoint;
                    newEdgePoint->position = (faceVert1->position + faceVert2->position
                                              + edgeVert1->position + edgeVert2->position)
                        / (float)4.0;
                }
                currEdge->edgePoint = newEdgePoint;
                newEdgePoint->ID = newVertList.size();
                newVertList.push_back(newEdgePoint);
                // cout<<"New Edge Point: ID: "<< newEdgePoint -> ID <<" Position: "<< (newEdgePoint
                // -> position)[0]<<" "<<(newEdgePoint -> position)[1]<<" "<<(newEdgePoint ->
                // position)[2]<<endl;
            }
            if (currFace == currEdge->fa)
            {
                currVert = currEdge->vb;
                currEdge = currEdge->nextVbFa;
            }
            else if (currFace == currEdge->fb)
            {
                if (currEdge->mobius)
                {
                    currVert = currEdge->vb;
                    currEdge = currEdge->nextVbFb;
                }
                else
                {
                    currVert = currEdge->va;
                    currEdge = currEdge->nextVaFb;
                }
            }
        } while (currEdge != firstEdge);
    }
}

void CMeshMerger::makeVertexPointsC(vector<Vertex*>& newVertList)
{
    vector<Vertex*>::iterator vIt;
    Vertex facePointAvg;
    Vertex edgePointAvg;
    Vertex* currVert;
    Vertex* newVertexPoint;
    for (vIt = currMesh.vertList.begin(); vIt < currMesh.vertList.end(); vIt++)
    {
        // cout<<"New Vertex!"<<endl;
        currVert = (*vIt);
        newVertexPoint = new Vertex;
        // cout<<"vertexID: "<<currVert -> ID<<endl;
        Edge* firstEdge = currVert->oneEdge;
        Edge* currEdge = firstEdge;
        Face* currFace = currEdge->fa;
        int sharpEdgeCounter = 0;
        Edge* sharpEdgeI;
        Edge* sharpEdgeK;
        Vector3 facePointAvgPosition = Vector3(0, 0, 0);
        Vector3 midPointAvgPoistion = Vector3(0, 0, 0);
        int n = 0;
        do
        {
            // cout<<"Now the sharp edge counter is "<<sharpEdgeCounter<<endl;
            // cout<<"here"<<endl<<nextOutEdge -> end -> ID<<endl;
            midPointAvgPoistion += (currEdge->va->position + currEdge->vb->position) / 2.0f;
            facePointAvgPosition += currFace->facePoint->position;
            n += 1;
            if (currEdge->isSharp)
            {
                // cout<<"A"<<endl;
                sharpEdgeCounter += 1;
                if (sharpEdgeCounter == 1)
                {
                    sharpEdgeI = currEdge;
                }
                else if (sharpEdgeCounter == 2)
                {
                    sharpEdgeK = currEdge;
                }
                currFace = currEdge->theOtherFace(currFace);
                if (currFace == NULL)
                {
                    // cout<<"A1"<<endl;
                    currEdge = currEdge->nextEdge(currVert, currFace);
                    currFace = currEdge->theOtherFace(currFace);
                    midPointAvgPoistion += (currEdge->va->position + currEdge->vb->position) / 2.0f;
                    sharpEdgeCounter += 1;
                    if (sharpEdgeCounter == 2)
                    {
                        sharpEdgeK = currEdge;
                    }
                }
                currEdge = currEdge->nextEdge(currVert, currFace);
            }
            else
            {
                currFace = currEdge->theOtherFace(currFace);
                currEdge = currEdge->nextEdge(currVert, currFace);
            }
        } while (currEdge != firstEdge);
        if (sharpEdgeCounter <= 1)
        {
            facePointAvgPosition /= n;
            midPointAvgPoistion /= n;
            newVertexPoint->position = ((float)(n - 3) * currVert->position
                                        + 2.0f * midPointAvgPoistion + facePointAvgPosition)
                / (float)n;
            // cout<<"this is a normal vertex! "<<newVertexPoint -> position[0] << newVertexPoint ->
            // position [1]<< newVertexPoint -> position[2]<<endl;
        }
        else if (sharpEdgeCounter == 2)
        {
            Vertex* pointI = sharpEdgeI->theOtherVertex(currVert);
            Vertex* pointK = sharpEdgeK->theOtherVertex(currVert);
            newVertexPoint->position =
                (pointI->position + pointK->position + 6.0f * currVert->position) / 8.0f;
            // cout<<"this is a crease vertex! "<<newVertexPoint -> position[0] << newVertexPoint ->
            // position [1]<< newVertexPoint -> position[2]<<endl;;
        }
        else
        {
            newVertexPoint->position = currVert->position;
            // cout<<"this is a conner vertex! "<<newVertexPoint -> position[0] << newVertexPoint ->
            // position [1]<< newVertexPoint -> position[2]<<endl;
        }
        newVertexPoint->ID = newVertList.size();
        currVert->vertexPoint = newVertexPoint;
        newVertList.push_back(newVertexPoint);
        // cout<<"New Vertex Point: ID: "<< newVertexPoint -> ID <<" Position: "<< (newVertexPoint
        // -> position)[0]<<" "<<(newVertexPoint -> position)[1]<<" "<<(newVertexPoint ->
        // position)[2]<<endl;
    }
}

void CMeshMerger::makeVertexPointsD(vector<Vertex*>& newVertList)
{
    vector<Vertex*>::iterator vIt;
    Vertex* currVert;
    Vertex* newVertexPoint;
    for (vIt = currMesh.vertList.begin(); vIt < currMesh.vertList.end(); vIt++)
    {
        // cout<<"New Vertex!"<<endl;
        currVert = (*vIt);
        newVertexPoint = new Vertex;
        // cout<<"vertexID: "<<currVert -> ID<<endl;
        Edge* firstEdge = currVert->oneEdge;
        Edge* currEdge = firstEdge;
        Face* currFace = currEdge->fa;
        int sharpEdgeCounter = 0;
        Edge* sharpEdgeI;
        Edge* sharpEdgeK;
        Vector3 facePointAvgPosition = Vector3(0, 0, 0);
        Vector3 edgePointAvgPoistion = Vector3(0, 0, 0);
        int n = 0;
        do
        {
            // cout<<"Now the sharp edge counter is "<<sharpEdgeCounter<<endl;
            // cout<<"here"<<endl<<nextOutEdge -> end -> ID<<endl;
            edgePointAvgPoistion += currEdge->edgePoint->position;
            facePointAvgPosition += currFace->facePoint->position;
            n += 1;
            if (currEdge->isSharp)
            {
                // cout<<"A"<<endl;
                sharpEdgeCounter += 1;
                if (sharpEdgeCounter == 1)
                {
                    sharpEdgeI = currEdge;
                }
                else if (sharpEdgeCounter == 2)
                {
                    sharpEdgeK = currEdge;
                }
                currFace = currEdge->theOtherFace(currFace);
                if (currFace == NULL)
                {
                    // cout<<"A1"<<endl;
                    currEdge = currEdge->nextEdge(currVert, currFace);
                    currFace = currEdge->theOtherFace(currFace);
                    edgePointAvgPoistion += currEdge->edgePoint->position;
                    sharpEdgeCounter += 1;
                    if (sharpEdgeCounter == 2)
                    {
                        sharpEdgeK = currEdge;
                    }
                }
                currEdge = currEdge->nextEdge(currVert, currFace);
            }
            else
            {
                currFace = currEdge->theOtherFace(currFace);
                currEdge = currEdge->nextEdge(currVert, currFace);
            }
        } while (currEdge != firstEdge);
        if (sharpEdgeCounter <= 1)
        {
            facePointAvgPosition /= n;
            edgePointAvgPoistion /= n;
            newVertexPoint->position =
                ((float)(n - 2) * currVert->position + edgePointAvgPoistion + facePointAvgPosition)
                / (float)n;
            // cout<<"this is a normal vertex! "<<newVertexPoint -> position[0] << newVertexPoint ->
            // position [1]<< newVertexPoint -> position[2]<<endl;
        }
        else if (sharpEdgeCounter == 2)
        {
            Vertex* pointI = sharpEdgeI->theOtherVertex(currVert);
            Vertex* pointK = sharpEdgeK->theOtherVertex(currVert);
            newVertexPoint->position =
                (pointI->position + pointK->position + 6.0f * currVert->position) / 8.0f;
            // cout<<"this is a crease vertex! "<<newVertexPoint -> position[0] << newVertexPoint ->
            // position [1]<< newVertexPoint -> position[2]<<endl;;
        }
        else
        {
            newVertexPoint->position = currVert->position;
            // cout<<"this is a conner vertex! "<<newVertexPoint -> position[0] << newVertexPoint ->
            // position [1]<< newVertexPoint -> position[2]<<endl;
        }
        newVertexPoint->ID = newVertList.size();
        currVert->vertexPoint = newVertexPoint;
        newVertList.push_back(newVertexPoint);
        // cout<<"New Vertex Point: ID: "<< newVertexPoint -> ID <<" Position: "<< (newVertexPoint
        // -> position)[0]<<" "<<(newVertexPoint -> position)[1]<<" "<<(newVertexPoint ->
        // position)[2]<<endl;
    }
}

void CMeshMerger::compileNewMesh(vector<Face*>& newFaceList)
{
    vector<Face*>::iterator fIt;
    for (fIt = currMesh.faceList.begin(); fIt < currMesh.faceList.end(); fIt++)
    {
        Face* currFace = (*fIt);
        Edge* firstEdge = currFace->oneEdge;
        Edge* currEdge = firstEdge;
        Edge* nextEdge;
        Edge* previousB;
        Edge* previousEF;
        Edge* edgeA;
        Edge* edgeB;
        Edge* edgeEF;
        Face* newFace;
        bool notFirstFace = false;
        // Split the edges and create "in and out" edges.`
        do
        {
            newFace = new Face;
            // Create edge and set va and vb
            if (currEdge->firstHalf == NULL)
            {
                Edge* newFirstHalf = new Edge;
                Edge* newSecondHalf = new Edge;
                newFirstHalf->va = currEdge->va->vertexPoint;
                newFirstHalf->vb = currEdge->edgePoint;
                newSecondHalf->va = currEdge->edgePoint;
                newSecondHalf->vb = currEdge->vb->vertexPoint;
                currEdge->firstHalf = newFirstHalf;
                currEdge->secondHalf = newSecondHalf;
                newFirstHalf->va->oneEdge = newFirstHalf;
                newSecondHalf->vb->oneEdge = newSecondHalf;
                currEdge->edgePoint->oneEdge = newFirstHalf;
            }
            edgeEF = new Edge;
            edgeEF->va = currEdge->edgePoint;
            edgeEF->vb = currFace->facePoint;
            if (notFirstFace)
            {
                edgeEF->fa = newFace;
                previousEF->fb = newFace;
            }
            if (currFace == currEdge->fa)
            {
                edgeA = currEdge->firstHalf;
                edgeB = currEdge->secondHalf;
                edgeA->nextVbFa = edgeEF;
                edgeB->nextVaFa = edgeEF;
                if (notFirstFace)
                {
                    edgeA->fa = newFace;
                    edgeA->nextVaFa = previousB;
                    if (previousB->vb == edgeA->va)
                    {
                        if (previousB->mobius)
                        {
                            previousB->nextVbFb = edgeA;
                            previousB->fb = newFace;
                        }
                        else
                        {
                            previousB->nextVbFa = edgeA;
                            previousB->fa = newFace;
                        }
                    }
                    else
                    {
                        previousB->nextVaFb = edgeA;
                        previousB->fb = newFace;
                    }
                }
                nextEdge = currEdge->nextVbFa;
                if (currEdge->fb == NULL)
                {
                    edgeA->nextVbFb = edgeB;
                    edgeB->nextVaFb = edgeA;
                    Edge* neighbourboundaryA = currEdge->nextVaFb;
                    Edge* neighbourboundaryB = currEdge->nextVbFb;
                    if (neighbourboundaryA->firstHalf != NULL && edgeA->nextVaFb == NULL)
                    {
                        if (neighbourboundaryA->vb == currEdge->va)
                        {
                            edgeA->nextVaFb = neighbourboundaryA->secondHalf;
                            neighbourboundaryA->secondHalf->nextVbFb = edgeA;
                        }
                        else
                        {
                            edgeA->nextVaFb = neighbourboundaryA->firstHalf;
                            neighbourboundaryA->firstHalf->nextVaFb = edgeA;
                        }
                    }
                    if (neighbourboundaryB->firstHalf != NULL && edgeB->nextVbFb == NULL)
                    {
                        if (neighbourboundaryB->va == currEdge->vb)
                        {
                            edgeB->nextVbFb = neighbourboundaryB->firstHalf;
                            neighbourboundaryB->firstHalf->nextVaFb = edgeB;
                        }
                        else
                        {
                            edgeB->nextVbFb = neighbourboundaryB->secondHalf;
                            neighbourboundaryB->secondHalf->nextVbFb = edgeB;
                        }
                    }
                }
            }
            else
            {
                if (currEdge->mobius)
                {
                    edgeA = currEdge->firstHalf;
                    edgeB = currEdge->secondHalf;
                    edgeA->mobius = true;
                    edgeB->mobius = true;
                    edgeA->va->onMobius = true;
                    edgeB->vb->onMobius = true;
                    edgeA->vb->onMobius = true;
                    nextEdge = currEdge->nextVbFb;
                    edgeA->nextVbFb = edgeEF;
                    edgeB->nextVaFb = edgeEF;
                    if (notFirstFace)
                    {
                        edgeA->fb = newFace;
                        edgeA->nextVaFb = previousB;
                        if (previousB->vb == edgeA->va)
                        {
                            if (previousB->mobius)
                            {
                                previousB->nextVbFb = edgeA;
                                previousB->fb = newFace;
                            }
                            else
                            {
                                previousB->nextVbFa = edgeA;
                                previousB->fa = newFace;
                            }
                        }
                        else
                        {
                            previousB->nextVaFb = edgeA;
                            previousB->fb = newFace;
                        }
                    }
                }
                else
                {
                    edgeA = currEdge->secondHalf;
                    edgeB = currEdge->firstHalf;
                    nextEdge = currEdge->nextVaFb;
                    edgeA->nextVaFb = edgeEF;
                    edgeB->nextVbFb = edgeEF;
                    if (notFirstFace)
                    {
                        edgeA->fb = newFace;
                        edgeA->nextVbFb = previousB;
                        if (previousB->vb == edgeA->vb)
                        {
                            if (previousB->mobius)
                            {
                                previousB->nextVbFb = edgeA;
                                previousB->fb = newFace;
                            }
                            else
                            {
                                previousB->nextVbFa = edgeA;
                                previousB->fa = newFace;
                            }
                        }
                        else
                        {
                            previousB->nextVaFb = edgeA;
                            previousB->fb = newFace;
                        }
                    }
                }
            }
            if (currEdge->isSharp)
            {
                edgeA->isSharp = true;
                edgeB->isSharp = true;
            }
            edgeEF->nextVaFa = edgeA;
            edgeEF->nextVaFb = edgeB;
            if (notFirstFace)
            {
                edgeEF->nextVbFa = previousEF;
                previousEF->nextVbFb = edgeEF;
            }
            currEdge = nextEdge;
            previousB = edgeB;
            previousEF = edgeEF;
            if (notFirstFace)
            {
                newFace->oneEdge = edgeA;
                newFace->id = newFaceList.size();
                newFaceList.push_back(newFace);
            }
            notFirstFace = true;
        } while (currEdge != firstEdge);
        newFace = new Face;
        previousEF->fb = newFace;
        if (currFace == currEdge->fa)
        {
            edgeA = currEdge->firstHalf;
            edgeEF = edgeA->nextVbFa;
            edgeA->fa = newFace;
            edgeA->nextVaFa = previousB;
            if (previousB->vb == edgeA->va)
            {
                if (previousB->mobius)
                {
                    previousB->nextVbFb = edgeA;
                    previousB->fb = newFace;
                }
                else
                {
                    previousB->nextVbFa = edgeA;
                    previousB->fa = newFace;
                }
            }
            else
            {
                previousB->nextVaFb = edgeA;
                previousB->fb = newFace;
            }
        }
        else
        {
            if (currEdge->mobius)
            {
                edgeA = currEdge->firstHalf;
                edgeEF = edgeA->nextVbFb;
                edgeA->fb = newFace;
                edgeA->nextVaFb = previousB;
                if (previousB->vb == edgeA->va)
                {
                    if (previousB->mobius)
                    {
                        previousB->nextVbFb = edgeA;
                        previousB->fb = newFace;
                    }
                    else
                    {
                        previousB->nextVbFa = edgeA;
                        previousB->fa = newFace;
                    }
                }
                else
                {
                    previousB->nextVaFb = edgeA;
                    previousB->fb = newFace;
                }
            }
            else
            {
                edgeA = currEdge->secondHalf;
                edgeEF = edgeA->nextVaFb;
                edgeA->fb = newFace;
                edgeA->nextVbFb = previousB;
                if (previousB->vb == edgeA->vb)
                {
                    if (previousB->mobius)
                    {
                        previousB->nextVbFb = edgeA;
                        previousB->fb = newFace;
                    }
                    else
                    {
                        previousB->nextVbFa = edgeA;
                        previousB->fa = newFace;
                    }
                }
                else
                {
                    previousB->nextVaFb = edgeA;
                    previousB->fb = newFace;
                }
            }
        }
        edgeEF->nextVbFa = previousEF;
        edgeEF->fa = newFace;
        previousEF->nextVbFb = edgeEF;
        newFace->oneEdge = edgeA;
        newFace->id = newFaceList.size();
        newFaceList.push_back(newFace);
        currFace->facePoint->oneEdge = previousEF;
    }
}

// Randy changed from DSMesh to void
void CMeshMerger::ccSubdivision(int level)
{
    cout << currMesh.faceList.size() << endl;
    DSMesh newMesh;
    for (int i = 0; i < level; i++)
    {
        makeFacePoints(newMesh.vertList);
        makeEdgePoints(newMesh.vertList);
        makeVertexPointsD(newMesh.vertList);
        compileNewMesh(newMesh.faceList);
        setAllNewPointPointersToNull();

        // Horribly messy. Organize this better in the future so subdivision cleans up all data structures accordingly
        for (int i = 0; i < newMesh.faceList.size(); i++)
        {
            Face* currFace = newMesh.faceList[i];
            std::string faceName = "subdivFace" + std::to_string(i);
            currFace->name = faceName;
            newMesh.nameToFace[faceName] = newMesh.faceList[i];
        }

        for (int i = 0; i < newMesh.vertList.size(); i++)
        {
            Vertex * currVert = newMesh.vertList[i];
            std::string vertName = "subdivVert" + std::to_string(i);
            currVert->name = vertName;
            newMesh.nameToVert[vertName] = newMesh.vertList[i];
        }

        currMesh.clear();
        currMesh = newMesh.randymakeCopy("", false); // Randy added this. Better copying than currMesh = newMesh
        newMesh.clear();
    }

    cout << "done catmull" << endl;
    cout << currMesh.faceList.size() << endl;
    
    //return currMesh;
}

void CMeshMerger::setAllNewPointPointersToNull()
{
    for (Vertex* v : currMesh.vertList)
    {
        v->vertexPoint = NULL;
    }
    for (Face* f : currMesh.faceList)
    {
        f->facePoint = NULL;
        Edge* firstEdge = f->oneEdge;
        Edge* currEdge = firstEdge;
        Vertex* currVert;
        do
        {
            currEdge->edgePoint = NULL;
            currEdge->firstHalf = NULL;
            currEdge->secondHalf = NULL;
            if (f == currEdge->fa)
            {
                currVert = currEdge->vb;
                currEdge = currEdge->nextVbFa;
            }
            else if (f == currEdge->fb)
            {
                if (currEdge->mobius)
                {
                    currVert = currEdge->vb;
                    currEdge = currEdge->nextVbFb;
                }
                else
                {
                    currVert = currEdge->va;
                    currEdge = currEdge->nextVaFb;
                }
            }
        } while (currEdge != firstEdge);
    }
}











//below brack is old
}