/**
 * @author Andy Wang, UC Berkeley.
 * Copyright 2016 reserve.
 * UC Berkeley, Slide_2016 project.
 * Advised by Prof. Sequin H. Carlos.
 */

//#include "group.h"
#include "DataStructureMesh.h"
//#include "parameter.h"
Mesh::Mesh(int type)
{
    user_set_color = false;
    //transformations_up.clear();
    parent = NULL;
    this->type = type;
    in_editing_mode = false;
    if (type == 1 || type == 2)
    {
        n = 0;
        ro = 0.0f;
        ratio = 0.0f;
        h = 0.0f;
        n_expr = "";
        ro_expr = "";
        ratio_expr = "";
        h_expr = "";
    }
    isConsolidateMesh = false;
}
void Mesh::addVertex(Vertex* v) { 
    vertList.push_back(v); 
    nameToVert[v->name] = v; // Randy added this
}

Edge* Mesh::findEdge(Vertex* v1, Vertex* v2)
{
    unordered_map<Vertex*, vector<Edge*>>::iterator vIt;
    vector<Edge*>::iterator eIt;
    vIt = edgeTable.find(v2);
    if (vIt != edgeTable.end())
    {
        for (eIt = (vIt->second).begin(); eIt < (vIt->second).end(); eIt++)
        {
            if ((*eIt)->vb == v1)
            {
                // cout<<"Find Edge from vertex "<<v2 -> ID<<" to vertex "<<v1 -> ID<<"."<<endl;
                return (*eIt);
            }
        }
    }
    vIt = edgeTable.find(v1);
    if (vIt != edgeTable.end())
    {
        for (eIt = (vIt->second).begin(); eIt < (vIt->second).end(); eIt++)
        {
            if ((*eIt)->vb == v2)
            {
                // cout<<"Find M Edge from vertex "<<v1 -> ID<<" to vertex "<<v2 -> ID<<"."<<endl;
                (*eIt)->mobius = true;
                (*eIt)->va->onMobius = true;
                (*eIt)->vb->onMobius = true;
                return (*eIt);
            }
        }
    }
    return NULL;
}

Edge* Mesh::createEdge(Vertex* v1, Vertex* v2)
{
    Edge* edge = findEdge(v1, v2);
    if (edge == NULL)
    {
        // cout<<"Creating new Edge from vertex "<<v1 -> ID<<" to vertex "<<v2 -> ID<<"."<<endl;
        edge = new Edge(v1, v2);
        if (v1->oneEdge == NULL)
        {
            v1->oneEdge = edge;
        }
        if (v2->oneEdge == NULL)
        {
            v2->oneEdge = edge;
        }
        unordered_map<Vertex*, vector<Edge*>>::iterator vIt;
        vIt = edgeTable.find(v1);
        if (vIt != edgeTable.end())
        {
            (vIt->second).push_back(edge);
        }
        else
        {
            vector<Edge*> currEdges;
            currEdges.push_back(edge);
            edgeTable[v1] = currEdges;
        }
    }
    // cout<<"The va of edge is "<<edge -> va -> ID<<" . The vb is "<< edge -> vb -> ID<<" ."<<endl;
    return edge;
}

void Mesh::addTriFace(Vertex* v1, Vertex* v2, Vertex* v3)
{
    Face* newFace = new Face({v1, v2, v3}); // Randy added the (vertices)
    Edge* e12 = createEdge(v1, v2);
    Edge* e23 = createEdge(v2, v3);
    Edge* e31 = createEdge(v3, v1);
    if (e12->fa == NULL)
    {
        e12->fa = newFace;
    }
    else if (e12->fb == NULL)
    {
        e12->fb = newFace;
    }
    else
    {
        cout << "ERROR: Try to create a Non-Manifold at edge with vertex1 : " << v1->ID
             << " and vertex2 :" << v2->ID << endl;
        exit(0);
    }
    if (e23->fa == NULL)
    {
        e23->fa = newFace;
    }
    else if (e23->fb == NULL)
    {
        e23->fb = newFace;
    }
    else
    {
        cout << "ERROR: Try to create a Non-Manifold at edge with vertex1 : " << v2->ID
             << " and vertex2 :" << v3->ID << endl;
        exit(0);
    }
    if (e31->fa == NULL)
    {
        e31->fa = newFace;
    }
    else if (e31->fb == NULL)
    {
        e31->fb = newFace;
    }
    else
    {
        cout << "ERROR: Try to create a Non-Manifold at edge with vertex1 : " << v3->ID
             << " and vertex2 :" << v1->ID << endl;
        exit(0);
    }
    newFace->oneEdge = e12;
    // cout<<"Testing: "<<newFace -> oneEdge -> va -> ID<<endl;
    e12->setNextEdge(v1, newFace, e31);
    e12->setNextEdge(v2, newFace, e23);
    e23->setNextEdge(v2, newFace, e12);
    e23->setNextEdge(v3, newFace, e31);
    e31->setNextEdge(v1, newFace, e12);
    e31->setNextEdge(v3, newFace, e23);
    newFace->id = faceList.size();
    faceList.push_back(newFace);
    nameToFace[newFace->name] = newFace; // Randy added this
}

void Mesh::addQuadFace(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4)
{
    Face* newFace = new Face({v1,v2,v3,v4});// Randy added the (vertices)
    Edge* e12 = createEdge(v1, v2);
    Edge* e23 = createEdge(v2, v3);
    Edge* e34 = createEdge(v3, v4);
    Edge* e41 = createEdge(v4, v1);
    if (e12->fa == NULL)
    {
        e12->fa = newFace;
    }
    else if (e12->fb == NULL)
    {
        e12->fb = newFace;
    }
    else
    {
        cout << "ERROR: Try to create a Non-Manifold at edge with vertex1 : " << v1->ID
             << " and vertex2 :" << v2->ID << endl;
        exit(0);
    }
    if (e23->fa == NULL)
    {
        e23->fa = newFace;
    }
    else if (e23->fb == NULL)
    {
        e23->fb = newFace;
    }
    else
    {
        cout << "ERROR: Try to create a Non-Manifold at edge with vertex1 : " << v2->ID
             << " and vertex2 :" << v3->ID << endl;
        exit(0);
    }
    if (e34->fa == NULL)
    {
        e34->fa = newFace;
    }
    else if (e34->fb == NULL)
    {
        e34->fb = newFace;
    }
    else
    {
        cout << "ERROR: Try to create a Non-Manifold at edge with vertex1 : " << v3->ID
             << " and vertex2 :" << v4->ID << endl;
        exit(0);
    }
    if (e41->fa == NULL)
    {
        e41->fa = newFace;
    }
    else if (e41->fb == NULL)
    {
        e41->fb = newFace;
    }
    else
    {
        cout << "ERROR: Try to create a Non-Manifold at edge with vertex1 : " << v4->ID
             << " and vertex2 :" << v1->ID << endl;
        exit(0);
    }
    newFace->oneEdge = e12;
    e12->setNextEdge(v1, newFace, e41);
    e12->setNextEdge(v2, newFace, e23);
    e23->setNextEdge(v2, newFace, e12);
    e23->setNextEdge(v3, newFace, e34);
    e34->setNextEdge(v3, newFace, e23);
    e34->setNextEdge(v4, newFace, e41);
    e41->setNextEdge(v4, newFace, e34);
    e41->setNextEdge(v1, newFace, e12);
    newFace->id = faceList.size();
    faceList.push_back(newFace);
    nameToFace[newFace->name] = newFace; // Randy added this
}

Face * Mesh::addPolygonFace(vector<Vertex*> vertices, bool reverseOrder)
{
    if (vertices.size() < 3)
    {
        cout << "A face have at least 3 vertices" << endl;
        return NULL; // Randy added the null
    }
    Face* newFace = new Face(vertices); // Randy added the (vertices)
    vector<Vertex*>::iterator vIt;
    vector<Edge*> edgesInFace;
    vector<Edge*>::iterator eIt;
    Edge* currEdge;

    if (!reverseOrder)
    {
        for (vIt = vertices.begin(); vIt < vertices.end(); vIt++)
        {
            if (vIt != vertices.end() - 1)
            {
                currEdge = createEdge(*vIt, *(vIt + 1));
                edgesInFace.push_back(currEdge);
            }
            else
            {
                currEdge = createEdge(*vIt, *(vertices.begin()));
                edgesInFace.push_back(currEdge);
            }
            if (currEdge->fa == NULL)
            {
                currEdge->fa = newFace;
            }
            else if (currEdge->fb == NULL)
            {
                currEdge->fb = newFace;
            }
            else
            {
                cout << "addPolygonFace ERROR: Try to create a Non-Manifold at edge with vertex1 : "
                     << currEdge->va->position.x << " " << currEdge->va->position.y << "  "
                     << currEdge->va->position.z << " and vertex2 :"
                    << currEdge->vb->position.x << " " << currEdge->vb->position.y << "  "
                                                     << currEdge->vb->position.z
                  << endl;
               // exit(0);
            }
        }
    }
    else
    {
        for (vIt = vertices.end() - 1; vIt >= vertices.begin(); vIt--)
        {
            if (vIt != vertices.begin())
            {
                currEdge = createEdge(*vIt, *(vIt - 1));
                edgesInFace.push_back(currEdge);
            }
            else
            {
                currEdge = createEdge(*vIt, *(vertices.end() - 1));
                edgesInFace.push_back(currEdge);
            }
            if (currEdge->fa == NULL)
            {
                currEdge->fa = newFace;
            }
            else if (currEdge->fb == NULL)
            {
                currEdge->fb = newFace;
            }
            else
            {
                cout << "ERROR: Try to create a Non-Manifold at edge with vertex1 : "
                     << currEdge->va->ID << " and vertex2 :" << currEdge->vb->ID << endl;
               // exit(0); // TODO: This is being triggered
            }
        }
    }
    newFace->oneEdge = currEdge;
    for (eIt = edgesInFace.begin(); eIt < edgesInFace.end(); eIt++)
    {
        Edge* currEdge = (*eIt);
        if (eIt == edgesInFace.begin())
        {
            if (newFace == currEdge->fa)
            {
                currEdge->nextVbFa = *(eIt + 1);
                currEdge->nextVaFa = *(edgesInFace.end() - 1);
            }
            else
            {
                if (currEdge->mobius)
                {
                    currEdge->nextVbFb = *(eIt + 1);
                    currEdge->nextVaFb = *(edgesInFace.end() - 1);
                }
                else
                {
                    currEdge->nextVaFb = *(eIt + 1);
                    currEdge->nextVbFb = *(edgesInFace.end() - 1);
                }
            }
        }
        else if (eIt == (edgesInFace.end() - 1))
        {
            if (newFace == currEdge->fa)
            {
                currEdge->nextVbFa = *(edgesInFace.begin());
                currEdge->nextVaFa = *(eIt - 1);
            }
            else
            {
                if (currEdge->mobius)
                {
                    currEdge->nextVbFb = *(edgesInFace.begin());
                    currEdge->nextVaFb = *(eIt - 1);
                }
                else
                {
                    currEdge->nextVaFb = *(edgesInFace.begin());
                    currEdge->nextVbFb = *(eIt - 1);
                }
            }
        }
        else
        {
            if (newFace == currEdge->fa)
            {
                currEdge->nextVbFa = *(eIt + 1);
                currEdge->nextVaFa = *(eIt - 1);
            }
            else
            {
                if (currEdge->mobius)
                {
                    currEdge->nextVbFb = *(eIt + 1);
                    currEdge->nextVaFb = *(eIt - 1);
                }
                else
                {
                    currEdge->nextVaFb = *(eIt + 1);
                    currEdge->nextVbFb = *(eIt - 1);
                }
            }
        }
    }
    newFace->id = faceList.size();
    faceList.push_back(newFace);
    nameToFace[newFace->name] = newFace; // Randy added this
    return newFace;
}

// Build the next pointers for boundary edges in the mesh.
// @param mesh: refer to the mesh to build connection in.
// This one takes O(E) time.
void Mesh::buildBoundary()
{
    unordered_map<Vertex*, vector<Edge*>>::iterator evIt;
    vector<Edge*> edgesAtThisPoint;
    vector<Edge*>::iterator eIt;
    for (evIt = edgeTable.begin(); evIt != edgeTable.end(); evIt++)
    {
        edgesAtThisPoint = evIt->second;
        if (!edgesAtThisPoint.empty())
        {
            for (eIt = edgesAtThisPoint.begin(); eIt < edgesAtThisPoint.end(); eIt++)
            {
                Edge* currEdge = (*eIt);
                if ((currEdge->nextEdge(currEdge->va, currEdge->fb)) == NULL)
                {
                    Edge* firstBoundaryEdge = currEdge;
                    Vertex* currVert = currEdge->va;
                    Edge* nextBoundaryEdge;
                    // cout<<"first: "<<currEdge -> va -> ID<<" "<<currEdge -> vb -> ID<<endl;
                    do
                    {
                        currEdge->isSharp = true;
                        // cout<<"Now building boundary at vertex: "<<endl;
                        // cout<<currVert -> ID<<endl;
                        Face* currFace = currEdge->fa;
                        nextBoundaryEdge = currEdge->nextEdge(currVert, currFace);
                        while (nextBoundaryEdge->fb != NULL)
                        {
                            currFace = nextBoundaryEdge->theOtherFace(currFace);
                            nextBoundaryEdge = nextBoundaryEdge->nextEdge(currVert, currFace);
                        }
                        currEdge->setNextEdge(currVert, NULL, nextBoundaryEdge);
                        nextBoundaryEdge->setNextEdge(currVert, NULL, currEdge);
                        currEdge = nextBoundaryEdge;
                        currVert = currEdge->theOtherVertex(currVert);
                    } while (currEdge != firstBoundaryEdge);
                }
            }
        }
    }
}

// @param p1, p2, p3 are positions of three vertices,
// with edge p1 -> p2 and edge p2 -> p3.
tc::Vector3 getNormal3Vertex(tc::Vector3 p1, tc::Vector3 p2, tc::Vector3 p3) { return (p2 - p1).CrossProduct(p3 - p2); }

// Get the surface normal.
// @param currFace: pointer of the face.
void getFaceNormal(Face* currFace)
{
    // cout<<"New Face!"<<endl;
    Edge* firstEdge = currFace->oneEdge;
    Edge* currEdge;
    Edge* nextEdge;
    currEdge = firstEdge;
    tc::Vector3 avgNorm = tc::Vector3(0, 0, 0);
    tc::Vector3 p1;
    tc::Vector3 p2;
    tc::Vector3 p3;
    do
    {
        // cout<<"New Edge!"<<endl;
        // cout<<"ID: "<<currEdge -> va -> ID<<endl;
        // cout<<"ID: "<<currEdge -> vb -> ID<<endl;
        if (currFace == currEdge->fa)
        {
            nextEdge = currEdge->nextVbFa;
            p1 = currEdge->va->position;
            p2 = currEdge->vb->position;
            p3 = nextEdge->theOtherVertex(currEdge->vb)->position;
        }
        else if (currFace == currEdge->fb)
        {
            if (currEdge->mobius)
            {
                nextEdge = currEdge->nextVbFb;
                p1 = currEdge->va->position;
                p2 = currEdge->vb->position;
                p3 = nextEdge->theOtherVertex(currEdge->vb)->position;
            }
            else
            {
                nextEdge = currEdge->nextVaFb;
                p1 = currEdge->vb->position;
                p2 = currEdge->va->position;
                p3 = nextEdge->theOtherVertex(currEdge->va)->position;
            }
        }
        avgNorm += getNormal3Vertex(p1, p2, p3);
        currEdge = nextEdge;
    } while (currEdge != firstEdge);
    // cout<<"The new Face normal is: "<<result[0]<<" "<<result[1]<<" "<<result[2]<<endl;
    currFace->normal = avgNorm.Normalized();

}

// Get the vertex normal
// @param currVert: the target vertex.
void getVertexNormal(Vertex* currVert)
{
    Edge* firstEdge = currVert->oneEdge;
    if (firstEdge == NULL)
    {   
        cout << currVert->name
         + " is a lonely vertex without any adjacent edges. This error message  also appear for shapes with non-manifold verts and for polylines"
            << endl;
        return;
    }
    Edge* currEdge = firstEdge;
    Face* currFace = currEdge->fa;
    tc::Vector3 avgNorm(0, 0, 0);
    int mobiusCounter = 0;
    do
    {
        if (mobiusCounter % 2 == 0)
        {
            avgNorm += currFace->normal;
        }
        else
        {
            avgNorm -= currFace->normal;
        }
        if (currEdge->mobius)
        {
            mobiusCounter += 1;
        }
        currFace = currEdge->theOtherFace(currFace);
        if (currFace == NULL)
        { // If the face is NULL, need to skip this face
            Edge* nextEdge = currEdge->nextEdge(currVert, currFace);
            if (nextEdge->va == currEdge->va || nextEdge->vb == currEdge->vb)
            {
                mobiusCounter += 1;
            }
            currEdge = nextEdge;
            currFace = currEdge->theOtherFace(currFace);
        }
        currEdge = currEdge->nextEdge(currVert, currFace);
    } while (currEdge != firstEdge);
    // if(currVert -> onMobius) {
    // cout<<"The value of avgNorm is :"<<avgNorm[0]<<" "<<avgNorm[1]<<" "<<avgNorm[2]<<endl;
    // cout<<"The position of this vertex is :"<<currVert -> position[0]<<" "<<currVert ->
    // position[1]<<" "<<currVert -> position[2]<<endl;
    //}//cout<<"ID: "<<currVert -> ID <<" has "<<mobiusCounter<<" mConter"<<endl;
    currVert->normal = avgNorm.Normalized();
}

// Iterate over every vertex in the mesh and compute its normal
void Mesh::computeNormals(bool isPolyline)
{
    vector<Vertex*>::iterator vIt;
    vector<Face*>::iterator fIt;
    // cout<<"faceTable size: "<<faceList.size()<<endl;
    for (fIt = faceList.begin(); fIt < faceList.end(); fIt++)
    {
        getFaceNormal(*fIt);
    }

    // cout<<"vertTable size: "<<vertList.size()<<endl;
    if (!isPolyline)
    {
        for (vIt = vertList.begin(); vIt != vertList.end(); vIt++)
        {
            // cout<<"Now calculating vertex with ID: "<< vIt -> first <<endl;
            getVertexNormal(*vIt);
        }
    }
}


vector<Edge*> Mesh::boundaryEdgeList()
{
    vector<Edge*> boundaryEdgeList;
    unordered_map<Vertex*, vector<Edge*>>::iterator vIt;
    // cout<<"Edge table size: "<<edgeTable.size()<<endl;
    for (vIt = edgeTable.begin(); vIt != edgeTable.end(); vIt++)
    {
        // cout<<vIt -> first -> ID<<endl;
        vector<Edge*> edges = vIt->second;
        vector<Edge*>::iterator eIt;
        vector<Edge*> newEdges;
        for (eIt = edges.begin(); eIt < edges.end(); eIt++)
        {
            if ((*eIt)->fb == NULL)
            {
                boundaryEdgeList.push_back(*eIt);
            }
        }
    }
    // cout<<"size: " << boundaryEdgeList.size()<<endl;
    return boundaryEdgeList;
}


bool Mesh::isEmpty() { return vertList.size() == 0 && faceList.size() == 0; }

void Mesh::clear()
{
     for(Vertex*& v : vertList)
    {
        delete v;
    }
     for(Face*& f : faceList)
    {
        delete f;
    }
    vertList.clear();
    faceList.clear();
    edgeTable.clear();
}

void Mesh::clearAndDelete()
{
    for (Vertex*& v : vertList)
    {
        delete v;
    }
    for (Face*& f : faceList)
    {
        delete f;
    }
    vertList.clear();
    faceList.clear();
    edgeTable.clear();
}

//void Mesh::setColor(QColor color) { this->color = color; }


// test function
Mesh Mesh::randymakeCopy(string copy_mesh_name, bool isPolyline)
{
    // cout<<"Creating a copy of the current map.\n";
    Mesh newMesh;

    if (copy_mesh_name == "")
    {
        newMesh.name = this->name;
    }
    else
    {
        newMesh.name = copy_mesh_name;
    }
    newMesh.clear();
    vector<Vertex*>::iterator vIt;
    for (vIt = vertList.begin(); vIt < vertList.end(); vIt++)
    {
        Vertex* vertCopy = new Vertex;
        vertCopy->ID = (*vIt)->ID;
        vertCopy->name = (*vIt)->name;
        vertCopy->position = (*vIt)->position;
        newMesh.addVertex(vertCopy);
    }
    vector<Face*>::iterator fIt;
    vector<Vertex*> vertices;
    for (fIt = faceList.begin(); fIt < faceList.end(); fIt++)
    {
        Face* tempFace = *fIt;
        Edge* firstEdge = tempFace->oneEdge;
        Edge* currEdge = firstEdge;
        Edge* nextEdge;
        Vertex* tempv;
        vertices.clear();
        do
        {
            if (tempFace == currEdge->fa)
            {
                tempv = currEdge->vb;
                nextEdge = currEdge->nextVbFa;
            }
            else
            {
                if (currEdge->mobius)
                {
                    tempv = currEdge->vb;
                    nextEdge = currEdge->nextVbFb;
                }
                else
                {
                    tempv = currEdge->va;
                    nextEdge = currEdge->nextVaFb;
                }
            }
            vertices.push_back(newMesh.vertList[tempv->ID]);
            currEdge = nextEdge;
        } while (currEdge != firstEdge);
        newMesh.addPolygonFace(vertices);
        newMesh.faceList[newMesh.faceList.size() - 1]->user_defined_color =
            (*fIt)->user_defined_color;
        newMesh.faceList[newMesh.faceList.size() - 1]->color = (*fIt)->color;
        newMesh.faceList[newMesh.faceList.size() - 1]->name = (*fIt)->name;
        newMesh.faceList[newMesh.faceList.size() - 1]->surfaceName =
            (*fIt)->surfaceName; // Randy added this
    }
    newMesh.buildBoundary();

    newMesh.computeNormals(isPolyline);

    return newMesh;
}

Mesh Mesh::makeCopy(string copy_mesh_name)
{
    // cout<<"Creating a copy of the current map.\n";
    Mesh newMesh;

    if (copy_mesh_name == "")
    {
        newMesh.name = this->name;
    }
    else
    {
        newMesh.name = copy_mesh_name;
    }
    newMesh.clear();
    vector<Vertex*>::iterator vIt;
    for (vIt = vertList.begin(); vIt < vertList.end(); vIt++)
    {
        Vertex* vertCopy = new Vertex;
        vertCopy->ID = (*vIt)->ID;
        vertCopy->name = (*vIt)->name;
        vertCopy->position = (*vIt)->position;
        vertCopy->isParametric = (*vIt)->isParametric;
        if ((*vIt)->isParametric)
        {
            vertCopy->x_expr = (*vIt)->x_expr;
            vertCopy->y_expr = (*vIt)->y_expr;
            vertCopy->z_expr = (*vIt)->z_expr;
            //vertCopy->params = (*vIt)->params;
            //vertCopy->influencingParams = (*vIt)->influencingParams;
        }
        newMesh.addVertex(vertCopy);
    }
    vector<Face*>::iterator fIt;
    vector<Vertex*> vertices;
    for (fIt = faceList.begin(); fIt < faceList.end(); fIt++)
    {
        Face* tempFace = *fIt;
        Edge* firstEdge = tempFace->oneEdge;
        Edge* currEdge = firstEdge;
        Edge* nextEdge;
        Vertex* tempv;
        vertices.clear();
        do
        {
            if (tempFace == currEdge->fa)
            {
                tempv = currEdge->vb;
                nextEdge = currEdge->nextVbFa;
            }
            else
            {
                if (currEdge->mobius)
                {
                    tempv = currEdge->vb;
                    nextEdge = currEdge->nextVbFb;
                }
                else
                {
                    tempv = currEdge->va;
                    nextEdge = currEdge->nextVaFb;
                }
            }
            vertices.push_back(newMesh.vertList[tempv->ID]);
            currEdge = nextEdge;
        } while (currEdge != firstEdge);
        newMesh.addPolygonFace(vertices);
        newMesh.faceList[newMesh.faceList.size() - 1]->user_defined_color =
            (*fIt)->user_defined_color;
        newMesh.faceList[newMesh.faceList.size() - 1]->color = (*fIt)->color;
        newMesh.faceList[newMesh.faceList.size() - 1]->name = (*fIt)->name;
        newMesh.faceList[newMesh.faceList.size() - 1]->surfaceName = (*fIt)->surfaceName; // Randy added this
    }
    newMesh.buildBoundary();
    newMesh.computeNormals();
    newMesh.params = params;
    newMesh.type = type;
    if (type == 1)
    {
        newMesh.n = n;
        newMesh.ro = ro;
        newMesh.ratio = ratio;
        newMesh.h = h;
        newMesh.n_expr = n_expr;
        newMesh.ro_expr = ro_expr;
        newMesh.ratio_expr = ratio_expr;
        newMesh.h_expr = h_expr;
        newMesh.influencingParams = influencingParams;
    }
    else if (type == 2)
    {
        newMesh.n = n;
        newMesh.ro = ro;
        newMesh.ratio = ratio;
        newMesh.h = h;
        newMesh.n_expr = n_expr;
        newMesh.ro_expr = ro_expr;
        newMesh.ratio_expr = ratio_expr;
        newMesh.h_expr = h_expr;
        newMesh.influencingParams = influencingParams;
    }
    return newMesh;
}

void Mesh::setGlobalParameter(unordered_map<string, Parameter>* params) { this->params = params; }

void Mesh::addParam(Parameter* param) { influencingParams.push_back(param); }

Vertex* Mesh::findVertexInThisMesh(string name)
{
    for (Vertex*& v : vertList)
    {
        // cout << v->name << endl;
        // cout << name << endl;
        if (v->name == name)
        {
            return v;
        }
    }
    return NULL;
}

bool Mesh::deleteFaceInThisMesh(string name)
{
    for (Face*& f : faceList)
    {
        // cout<<f->name<<" ";
        if (f->name == name)
        {
            deleteFace(f);
            return true;
        }
    }
    return false;
}

void Mesh::deleteVertex(Vertex* v)
{
    bool foundThisVertex = false;
    int counter = 0;
    for (Vertex* nextVert : vertList)
    {
        if (foundThisVertex)
        {
            nextVert->ID -= 1;
        }
        else if (nextVert == v)
        {
            foundThisVertex = true;
        }
        else
        {
            counter++;
        }
    }
    if (foundThisVertex)
    {
        vertList.erase(vertList.begin() + counter);
    }
    delete v;
}

void Mesh::deleteFace(Face* face)
{
    bool foundThisFace = false;
    int counter = 0;
    for (Face* nextFace : faceList)
    {
        if (foundThisFace)
        {
            nextFace->id -= 1;
        }
        else if (nextFace == face)
        {
            foundThisFace = true;
        }
        else
        {
            counter++;
        }
    }
    if (foundThisFace)
    {
        faceList.erase(faceList.begin() + counter);
    }
    vector<Edge*> removeEdgeList;
    removeEdgeList.clear();
    Edge* firstEdge = face->oneEdge;
    Edge* currEdge = firstEdge;
    Edge* nextEdge;
    do
    {
        if (face == currEdge->fa)
        {
            nextEdge = currEdge->nextVbFa;
            if (currEdge->fb != NULL)
            {
                if (currEdge->mobius)
                {
                    currEdge->nextVaFa = currEdge->nextVaFb;
                    currEdge->nextVbFa = currEdge->nextVbFb;
                    currEdge->mobius = false;
                    // Maybe we need to remark the end vertex non-Mobius here.
                }
                else
                {
                    /*Switch the va and vb, also need to change in the edgetable. */
                    unordered_map<Vertex*, vector<Edge*>>::iterator vIt;
                    vector<Edge*>::iterator eIt;
                    vIt = edgeTable.find(currEdge->va);
                    if (vIt != edgeTable.end())
                    {
                        for (eIt = (vIt->second).begin(); eIt < (vIt->second).end(); eIt++)
                        {
                            if ((*eIt) == currEdge)
                            {
                                (vIt->second).erase(eIt);
                                break;
                            }
                        }
                    }
                    else
                    {
                        cout << "Error, there is a bug in the program!" << endl;
                    }
                    vIt = edgeTable.find(currEdge->vb);
                    if (vIt != edgeTable.end())
                    {
                        (vIt->second).push_back(currEdge);
                    }
                    else
                    {
                        vector<Edge*> edges;
                        edges.push_back(currEdge);
                        edgeTable[currEdge->vb] = edges;
                    }
                    Vertex* temp = currEdge->va;
                    currEdge->va = currEdge->vb;
                    currEdge->vb = temp;
                    currEdge->nextVaFa = currEdge->nextVbFb;
                    currEdge->nextVbFa = currEdge->nextVaFb;
                }
                currEdge->fa = currEdge->fb;
                currEdge->nextVaFb = NULL;
                currEdge->nextVbFb = NULL;
                currEdge->fb = NULL;
            }
            else
            {
                currEdge->nextVaFa = NULL;
                currEdge->nextVbFa = NULL;
                currEdge->fa = NULL;
                removeEdgeList.push_back(currEdge);
            }
        }
        else
        {
            if (currEdge->mobius)
            {
                nextEdge = currEdge->nextVbFb;
                currEdge->mobius = false;
            }
            else
            {
                nextEdge = currEdge->nextVaFb;
            }
            currEdge->nextVaFb = NULL;
            currEdge->nextVbFb = NULL;
            currEdge->fb = NULL;
        }
        currEdge = nextEdge;
    } while (currEdge != firstEdge);
    for (Edge* edge : removeEdgeList)
    {
        deleteEdge(edge);
    }
    delete face;
    return;
}

void Mesh::deleteEdge(Edge* edge)
{
    unordered_map<Vertex*, vector<Edge*>>::iterator vIt;
    vector<Edge*>::iterator eIt;
    vIt = edgeTable.find(edge->va);
    bool foundEdge = false;
    if (vIt != edgeTable.end())
    {
        for (eIt = (vIt->second).begin(); eIt < (vIt->second).end(); eIt++)
        {
            if ((*eIt) == edge)
            {
                foundEdge = true;
                (vIt->second).erase(eIt);
                break;
            }
        }
    }
    if (!foundEdge)
    {
        cout << "Error: You can't delete this edge. Check the program!" << endl;
    }
    else
    {
        /* Also need to settle edgeTable here. We also need to set the oneEdge pointer
         * for the affected vertex when edge is deleted. */
        if (edge->va->oneEdge == edge)
        {
            foundEdge = false;
            if ((vIt->second).size() > 0)
            {
                edge->va->oneEdge = *((vIt->second).begin());
                foundEdge = true;
            }
            else
            {
                for (vIt = edgeTable.begin(); vIt != edgeTable.end(); vIt++)
                {
                    for (eIt = (vIt->second).begin(); eIt < (vIt->second).end(); eIt++)
                    {
                        if ((*eIt)->vb == edge->va)
                        {
                            edge->va->oneEdge = (*eIt);
                            foundEdge = true;
                            break;
                        }
                    }
                }
            }
            if (!foundEdge)
            {
                cout << "Warning: Your deletion has deleted a vertex." << endl;
                edge->va->oneEdge = NULL;
                deleteVertex(edge->va);
            }
        }
        if (edge->vb->oneEdge == edge)
        {
            foundEdge = false;
            vIt = edgeTable.find(edge->vb);
            if (vIt != edgeTable.end() && (vIt->second).size() > 0)
            {
                edge->vb->oneEdge = *((vIt->second).begin());
                foundEdge = true;
            }
            else
            {
                for (vIt = edgeTable.begin(); vIt != edgeTable.end(); vIt++)
                {
                    for (eIt = (vIt->second).begin(); eIt < (vIt->second).end(); eIt++)
                    {
                        if ((*eIt)->vb == edge->vb)
                        {
                            edge->vb->oneEdge = (*eIt);
                            foundEdge = true;
                            break;
                        }
                    }
                }
            }
            if (!foundEdge)
            {
                cout << "Warning: Your deletion has deleted a vertex." << endl;
                edge->vb->oneEdge = NULL;
                deleteVertex(edge->vb);
            }
        }
    }

    /* Also need to remove the key in edgeTable if it does not exist anymore. */
    delete (edge);
    return;
}

void Mesh::updateVertListAfterDeletion()
{
    vertList.clear();
    bool foundVa;
    bool foundVb;
    unordered_map<Vertex*, vector<Edge*>>::iterator vIt;
    for (vIt = edgeTable.begin(); vIt != edgeTable.end(); vIt++)
    {
        for (Edge* edge : (vIt->second))
        {
            foundVa = false;
            foundVb = false;
            for (Vertex* v : vertList)
            {
                if (v == edge->va)
                {
                    foundVa = true;
                }
                if (v == edge->vb)
                {
                    foundVb = true;
                }
                if (foundVa && foundVb)
                {
                    break;
                }
            }
            if (!foundVa)
            {
                vertList.push_back(edge->va);
            }
            if (!foundVb)
            {
                vertList.push_back(edge->vb);
            }
        }
    }
    return;
}

void Mesh::setBoundaryEdgeToNull(Vertex* v)
{
    /* Traverse around v and find boundary edges.
     * Set the nextFb pointers to NULL for those edges. */
    vector<Edge*> edgeAtThisPoint;
    unordered_map<Vertex*, vector<Edge*>>::iterator vIt;
    vIt = edgeTable.find(v);
    if (vIt != edgeTable.end())
    {
        edgeAtThisPoint = vIt->second;
        for (Edge* e : edgeAtThisPoint)
        {
            e->isSharp = false;
            if ((e->fb) == NULL)
            {
                e->nextVaFb = NULL;
                e->nextVbFb = NULL;
            }
        }
    }
    else
    {
        cout << "Error: The Vertex doesn't belongs to this Mesh. Debug here." << endl;
    }
}
