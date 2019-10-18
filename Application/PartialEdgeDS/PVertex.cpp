//
// Created by 殷子欣 on 2019/10/14.
//

#include "PVertex.h"
#include "Vertex.h"
#include "Edge.h"
#include "PEdge.h"

namespace Nome::PartialEdgeDS
{

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
            tempEdges = this->visitAndGetEdges(nextEdge);
            edges.insert(edges.end(), tempEdges.begin(), tempEdges.end());
        }
    }
    return edges;
}

} /* namespace Nome::Scene::PartialEdgeDS */