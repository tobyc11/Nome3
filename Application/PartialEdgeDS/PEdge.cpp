//
// Created by 殷子欣 on 2019/10/14.
//

#include "PEdge.h"
#include "PVertex.h"
#include "Loop.h"
#include "Edge.h"

namespace Nome::PartialEdgeDS
{

PEdge::PEdge(const Loop *&loop, const Entity *&child, const EType &type, const PVertex *&pVertex, \
             const PEdge *&loopedNext, const PEdge *&loopedPrev, const PEdge *&radialNext, const PEdge *&radialPrev) : \
          Entity(), loop(loop), child(child), type(type), pVertex(pVertex), loopedNext(loopedNext), \
          loopedPrev(loopedPrev), radialNext(radialNext), radialPrev(radialPrev)
{
    if (loopedPrev != NULL) { loopPrev->loopNext = this; }
    if (loopedNext != NULL) { loopNext->loopPrev = this; }
    if (radialPrev != NULL) { radialPrev->radialNext = this; }
    if (radialNext != NULL) { radialNext->radialPrev = this; }
}


Loop *loop;
Entity *child; // Can be a edge, or vertex
EType type;
// Extra Information
//TODO: Add Orientation Flag
PVertex *pVertex; // Start pvertex
PEdge *loopedPrev;
PEdge *loopedNext;
PEdge *radialPrev;
PEdge *radialNext;

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