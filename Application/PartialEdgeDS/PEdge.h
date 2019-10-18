//
// Created by 殷子欣 on 2019/10/14.
//

#pragma once
#include "Entity.h"

namespace Nome::PartialEdgeDS
{

class Loop;
class PVertex;

class PEdge: public Entity
{
public:
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
};

} /* namespace Nome::Scene::PartialEdgeDS */
