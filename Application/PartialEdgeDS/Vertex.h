//
// Created by 殷子欣 on 2019/10/14.
//

#pragma once
#include "Entity.h"

namespace Nome::PartialEdgeDS
{

class Vertex: public Entity
{
public:
    Entity *parent; // Can be a partial face, partial edge,
    // or partial vertex
    //TODO: Add Point Geometry
};

} /* namespace Nome::Scene::PartialEdgeDS */