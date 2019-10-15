//
// Created by 殷子欣 on 2019/10/14.
//

#pragma once
#include "Entity.h"

namespace Nome::PartialEdgeDS
{

class PVertex;

class Edge : public Entity
{
public:
    Entity* parent; // Can be a partial face, or partial edge
    PVertex* pvertices[2];
    // TODO: Add Curve Geometry
};

} /* namespace Nome::Scene::PartialEdgeDS */