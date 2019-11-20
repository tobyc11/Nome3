//
// Created by 殷子欣 on 2019/10/14.
//

#include "PVertex.h"
#include "Vertex.h"
#include "Edge.h"
#include "PEdge.h"

namespace Nome::PartialEdgeDS
{

PVertex::PVertex(const Entity *&parent, const EType &type, const PVertex *&next, const PVertex *&prev, const Vertex *&vertex) : \
          Entity(), parent(parent), type(type), next(next), prev(prev), vertex(vertex)
{
    if (prev != NULL) { prev->next = this; }
    if (next != NULL) { next->prev = this; }
}

PVertex::~PVertex()
{
    if (prev == NULL && next == NULL && parent != NULL) {
        delete(parent);
        return;
    }

    switch (type) {
    case EType::EDGE :
        Edge *edge = (Edge *)parent;
        if (prev != NULL)
        {
            prev->next = NULL;
            edge->pVertices[1] = NULL;
        } else {
            edge->pVertices[0] = NULL;
            next->prev = NULL;
        }
        break;
    case EType::PEDGE :
        if (prev != NULL)
        {
            prev->next = next;
        } else {
            ((PEdge *)parent)->child = next;
        }

        if (next != NULL) { next->prev = prev; }
        break;
    case default :
        break;
    }

}

void PVertex::killChildren() {
    if (vertex!= NULL)
    {
        vertex->parent = NULL;
        delete(vertex);
    }
}

std::vector<Edge *> PVertex::getAndVisitEdges(Edge *edge) const {
    std::vector<Edge *> edges, tempEdges;
    PEdge *tempPEdge;
    Edge *nextEdge;

    edges.push_back(edge);
    edge->isVisit = true;

    /** how to check if it is a wire edge?
     * if edges == wire edge { return edges; }
     */

    if (edge->type != EType::PEDGE) { return edges; }

    tempPEdge = (PEdge *)edge->parent;

    // how to visit each p-edge around edge
    while (tempPEdge != NULL) {
        if (tempPEdge->pVertex == this) {
            if (tempPEdge->loopedPrev->type != EType::EDGE) { continue; }
            nextEdge = (Edge *)tempPEdge->loopedPrev->child;
        } else {
            if (tempPEdge->loopedNext->type != EType::EDGE) { continue; }
            nextEdge = (Edge *)tempPEdge->loopedNext->child;
        }
        if (nextEdge->isVisit == false) {
            tempEdges = this->getAndVisitEdges(nextEdge);
            edges.insert(edges.end(), tempEdges.begin(), tempEdges.end());
        }
    }
    return edges;
}

} /* namespace Nome::Scene::PartialEdgeDS */