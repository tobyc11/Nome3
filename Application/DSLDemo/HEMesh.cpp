#include "HEMesh.h"

namespace Nome
{

CHEMesh::CHEMesh()
{
}

CHEMesh::Vertex* CHEMesh::AllocateVertex(const Vector3 & position)
{
    void* mem = VertexPool.Allocate();
    auto* ptr = new(mem) Vertex();
    ptr->Position = position;
    return ptr;
}

CHEMesh::Edge* CHEMesh::AllocateEdge()
{
    void* mem = EdgePool.Allocate();
    auto* ptr = new(mem) Edge();
    return ptr;
}

CHEMesh::Face* CHEMesh::AllocateFace()
{
    void* mem = FacePool.Allocate();
    auto* ptr = new(mem) Face();
    return ptr;
}

CHEMesh::HalfEdge* CHEMesh::AllocateHalfEdge()
{
    void* mem = HalfEdgePool.Allocate();
    auto* ptr = new(mem) HalfEdge();
    return ptr;
}

void CHEMesh::FreeVertex(Vertex* p)
{
    p->~Vertex();
    VertexPool.Free(p);
}

void CHEMesh::FreeEdge(Edge* p)
{
    p->~Edge();
    EdgePool.Free(p);
}

void CHEMesh::FreeFace(Face* p)
{
    p->~Face();
    FacePool.Free(p);
}

void CHEMesh::FreeHalfEdge(HalfEdge* p)
{
    p->~HalfEdge();
    HalfEdgePool.Free(p);
}

CHEMesh::Vertex* CHEMesh::MakeVertex(const Vector3& position)
{
    Vertex* v = AllocateVertex(position);
    Vertices.insert(v);
    return v;
}

CHEMesh::Edge* CHEMesh::MakeEdgeVertex(Vertex* v0, const Vector3& position, Face* left, Face* right)
{
    //If isolated vertex
    if (v0->OneHE == nullptr)
    {
        Vertex* v1 = AllocateVertex(position);
        Edge* edge = AllocateEdge();
        HalfEdge* v0v1;
        HalfEdge* v1v0;
        CreateHalfEdgePair(v0v1, v1v0);
        Face* boundry = AllocateFace(); boundry->bIsBoundary = true;
        v0v1->Edge = v1v0->Edge = edge;
        v0v1->Vert = v0;
        v1v0->Vert = v1;
        v0v1->Face = boundry;
        v1v0->Face = boundry;

        v0v1->Next = v1v0;
        v1v0->Next = v0v1;

        edge->OneHE = v0v1;
        v0->OneHE = v0v1;
        v1->OneHE = v1v0;
        boundry->OneHE = v0v1;

        Edges.insert(edge);
        Vertices.insert(v1);
        BoundaryFaces.insert(boundry);
        return edge;
    }
    else if (left == right)
    {
        //We got a single faced graph, so just add one vertex and attach
        Vertex* v1 = AllocateVertex(position);
        Edge* edge = AllocateEdge();
        HalfEdge* v0v1;
        HalfEdge* v1v0;
        CreateHalfEdgePair(v0v1, v1v0);
        v0v1->Edge = v1v0->Edge = edge; edge->OneHE = v0v1;

        HalfEdge* preHe = v0->OneHE;
        HalfEdge* postHe = v0->OneHE->Twin->Next;

        preHe->Twin->Next = v0v1;
        v0v1->Next = v1v0;
        v1v0->Next = postHe;

        v0v1->Vert = v0; v0->OneHE = v0v1;
        v1v0->Vert = v1; v1->OneHE = v1v0;
        v0v1->Face = left;
        v1v0->Face = left;

        Edges.insert(edge);
        Vertices.insert(v1);
        return edge;
    }

    return nullptr;
}

CHEMesh::Edge* CHEMesh::MakeEdgeFace(Face* face, Vertex* v0, Vertex* v1)
{
    //Suppose we want edge from v0->v1
    HalfEdge* cursor = face->OneHE;
    while (cursor->Vert != v1)
        cursor = cursor->Next;

    HalfEdge* v1Out = cursor;
    HalfEdge* v1In = PrevAroundVertex(v1Out)->Twin;
    assert(v1Out->Face == face);

    while (cursor->Twin->Vert != v0)
        cursor = cursor->Next;
    assert(cursor->Face == face);
    HalfEdge* v0In = cursor;
    HalfEdge* v0Out = v0In->Next;

    Face* newFace = AllocateFace();
    Faces.insert(newFace);

    Edge* edge = AllocateEdge();
    Edges.insert(edge);

    HalfEdge* fwd;
    HalfEdge* bkwd;
    CreateHalfEdgePair(fwd, bkwd);

    fwd->Edge = bkwd->Edge = edge;
    fwd->Face = newFace; bkwd->Face = face;
    fwd->Vert = v0; bkwd->Vert = v1;
    fwd->Next = v1Out; bkwd->Next = v0Out;

    v1In->Next = bkwd;
    v0In->Next = fwd;

    newFace->OneHE = fwd;
    edge->OneHE = fwd;

    return edge;
}

void CHEMesh::CreateHalfEdgePair(HalfEdge*& outPtr, HalfEdge*& outPtr2)
{
    outPtr = AllocateHalfEdge();
    outPtr2 = AllocateHalfEdge();
    outPtr->Twin = outPtr2;
    outPtr2->Twin = outPtr;
    HalfEdges.insert(outPtr);
    HalfEdges.insert(outPtr2);
}

std::vector<CHEMesh::Vertex*> CHEMesh::VVQuery(Vertex* v)
{
    std::vector<Vertex*> result;
    HalfEdge* he = v->OneHE;
    do
    {
        result.push_back(he->Twin->Vert);
        he = he->Twin->Next;
    } while (he != v->OneHE);
    return result;
}

std::vector<CHEMesh::Edge*> CHEMesh::VEQuery(Vertex* v)
{
    std::vector<Edge*> result;
    HalfEdge* he = v->OneHE;
    do
    {
        result.push_back(he->Edge);
        he = he->Twin->Next;
    } while (he != v->OneHE);
    return result;
}

std::vector<CHEMesh::Face*> CHEMesh::VFQuery(Vertex* v)
{
    std::vector<Face*> result;
    HalfEdge* he = v->OneHE;
    do
    {
        result.push_back(he->Face);
        he = he->Twin->Next;
    } while (he != v->OneHE);
    return result;
}

std::vector<CHEMesh::Vertex*> CHEMesh::EVQuery(Edge* e)
{
    return { e->OneHE->Vert, e->OneHE->Twin->Vert };
}

std::vector<CHEMesh::Edge*> CHEMesh::EEQuery(Edge* e)
{
    return std::vector<Edge*>();
}

std::vector<CHEMesh::Face*> CHEMesh::EFQuery(Edge* e)
{
    return { e->OneHE->Face, e->OneHE->Twin->Face };
}

std::vector<CHEMesh::Vertex*> CHEMesh::FVQuery(Face* f)
{
    return std::vector<Vertex*>();
}

std::vector<CHEMesh::Edge*> CHEMesh::FEQuery(Face* f)
{
    return std::vector<Edge*>();
}

std::vector<CHEMesh::Face*> CHEMesh::FFQuery(Face* f)
{
    return std::vector<Face*>();
}

CHEMesh::HalfEdge* CHEMesh::NextAroundVertex(HalfEdge* he)
{
    return he->Twin->Next;
}

CHEMesh::HalfEdge* CHEMesh::PrevAroundVertex(HalfEdge* he)
{
    HalfEdge* last = he;
    HalfEdge* next = NextAroundVertex(he);
    while (next != he)
    {
        last = next;
        next = NextAroundVertex(next);
    }
    return last;
}

}
