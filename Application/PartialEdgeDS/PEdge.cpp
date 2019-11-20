//
// Created by 殷子欣 on 2019/10/14.
//

#include "PEdge.h"
#include "PVertex.h"
#include "Loop.h"
#include "Edge.h"

namespace Nome::PartialEdgeDS
{
PEdge::~PEdge()
{
    if (loopedPrev == NULL && loopedNext == NULL && loop != NULL) {
        delete(loop);
        return;
    }

    if (loopedPrev != NULL)
    {
        loopedPrev->loopedNext = loopedNext;
    } else {
        loop->pEdge = loopedNext;
    }

    if (loopedNext != NULL) { loopedNext->loopedPrev = loopedPrev; }

    if (radialPrev != NULL) { radialPrev->radialNext = radialNext; }
    if (radialNext != NULL) { radialNext->radialPrev = radialPrev; }

}

void PEdge::killChildren() {

    if (child != NULL)
    {
        child->killChildren();
        switch (type) {
        case EType::EDGE :
            ((Edge *)child)->parent = NULL;
            delete(child);
            break;
        case EType::PVERTEX :
            PVertex *tempPVertex = (PVertex *)child, *deletePVertex;
            while (tempPVertex != NULL)
            {
                tempPVertex->killChildren();
                tempPVertex->parent = NULL;
                deletePVertex = tempPVertex;
                tempPVertex = tempPVertex->next;
                delete(deletePVertex);
            }
            break;
        case default:
            delete(child);
            break;
        }
    }
}

} /* namespace Nome::Scene::PartialEdgeDS */