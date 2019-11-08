#include "QuadEdge.h"

namespace Nome::Scene
{

void CQEMesh::CEdge::Splice(CQEMesh::CEdge* a, CQEMesh::CEdge* b)
{
    assert(a);
    assert(b);

    CEdge* alpha = a->Onext()->Rot();
    CEdge* beta = b->Onext()->Rot();

    std::swap(a->Next, b->Next);
    std::swap(alpha->Next, beta->Next);
}

CQEMesh::CQuadEdge::CQuadEdge(uint32_t id)
{
    // Two vertices and same face
    Comp[0].SetNext(&Comp[0]);
    Comp[2].SetNext(&Comp[2]);
    Comp[1].SetNext(&Comp[3]);
    Comp[3].SetNext(&Comp[1]);
    Comp[0].SetId(id);
    Comp[1].SetId(id);
    Comp[2].SetId(id);
    Comp[3].SetId(id);
    Comp[0].SetIndex(0);
    Comp[1].SetIndex(1);
    Comp[2].SetIndex(2);
    Comp[3].SetIndex(3);
}

void CQEMesh::MakeEdge2V1F()
{
    auto* qe = new CQuadEdge(NextQuadEdgeId);
    ++NextQuadEdgeId;

    uint32_t iv1 = Vertices.size();
    Vertices.push_back(CVertex());
    uint32_t iv2 = iv1 + 1;
    Vertices.push_back(CVertex());

    auto* edge = qe->Comp;
}

}
