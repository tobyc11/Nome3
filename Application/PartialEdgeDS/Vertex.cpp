//
// Created by 殷子欣 on 2019/10/14.
//

#include "Vertex.h"
#include "Edge.h"
#include "PVertex.h"

namespace Nome::PartialEdgeDS
{

std::vector<Edge *> Vertex::getEdges() const {

    std::vector<Edge *> edges, tempEdges;
    if (type != EType::PVERTEX) { return edges; }

    PVertex *tempPVertex = (PVertex *)parent;
    while (tempPVertex != NULL) {
        tempEdges = tempPVertex->getAndVisitEdges(tempPVertex->edge);
        edges.insert(edges.end(), tempEdges.begin(), tempEdges.end());
        tempPVertex = tempPVertex->next;
    }

    for (std::vector<Edge *>::iterator it = edges.begin(); it != edges.end(); it++) { (*it)->isVisit = false; }

    return edges;
}

Vertex* Vertex::killVertex() {
    if (type == EType::PFACE)
    {
        delete(point);
        parent
    }
}

} /* namespace Nome::Scene::PartialEdgeDS */