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

    // Extra Information
    //TODO: Add Orientation Flag
    PVertex *pvertex; // Start pvertex
    PEdge *looped_prev;
    PEdge *looped_next;
    PEdge *radial_prev;
    PEdge *radial_next;
};

} /* namespace Nome::Scene::PartialEdgeDS */
