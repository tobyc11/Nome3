//
// Created by 殷子欣 on 2019/10/14.
//

#include "Face.h"
#include "PFace.h"
#include "PEdge.h"
#include "Loop.h"

namespace Nome::PartialEdgeDS
{

Face::Face(const PFace *&pFace, const Loop *&loop, const Geometry *&geometry) : \
           Entity(), pFace(pFace), loop(loop), geometry(geometry)
{

}

Face::~Face()
{
    if (geometry != NULL) { delete(geometry); }
    if (pFace != NULL) { delete(pFace); }
}

void Face::killChildren() {

    Loop *tempLoop = loop, *deleteLoop;
    while (tempLoop != NULL)
    {
        tempLoop->killChildren();
        tempLoop->face = NULL;
        deleteLoop = tempLoop;
        tempLoop = tempLoop->next;
        delete(deleteLoop);
    }
}

std::vector<Edge *> Face::getEdges() const {
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