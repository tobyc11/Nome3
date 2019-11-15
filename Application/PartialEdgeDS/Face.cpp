//
// Created by 殷子欣 on 2019/10/14.
//

#include "Face.h"
#include "PFace.h"
#include "PEdge.h"
#include "Loop.h"

namespace Nome::PartialEdgeDS
{
std::vector<Edge *> getEdges() const {
    std::vector<Edge *> edges;
    Loop *tempLoop = loop;

    while (tempLoop != NULL)
    {
        PEdge *tempPEdge = tempLoop->pEdge;
        while (tempPEdge != NULL)
        {
           if (tempPEdge->child != nil && tempPEdge->type == EType::EDGE) { edges.push_back(tempPEdge->child); }
           tempPEdge = tempPEdge->loopedNext;
        }
        tempLoop = tempLoop->next;
    }
    return edges;
}

} /* namespace Nome::Scene::PartialEdgeDS */