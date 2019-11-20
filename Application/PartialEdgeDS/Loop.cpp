//
// Created by 殷子欣 on 2019/10/14.
//

#include "Loop.h"
#include "PEdge.h"
#include "Face.h"

namespace Nome::PartialEdgeDS
{

Loop::~Loop()
{
    if (prev == NULL && next == NULL && face != NULL) {
        delete(face);
        return;
    }

    if (prev != NULL)
    {
        prev->next = next;
    } else {
        face->loop = next;
    }

    if (next != NULL) { next->prev = prev; }

}

void Loop::killChildren() {

    PEdge *tempPEdge = pEdge, *deletePEdge;
    while (tempPEdge != NULL)
    {
        tempPEdge->killChildren();
        tempPEdge->loop = NULL;
        deletePEdge = tempPEdge;
        tempPEdge = tempPEdge->loopedNext;
        delete(deletePEdge);
    }
}

} /* namespace Nome::Scene::PartialEdgeDS */