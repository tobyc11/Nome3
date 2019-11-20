//
// Created by 殷子欣 on 2019/10/14.
//

#pragma once
#include "Entity.h"
#include <vector>
#include "Geometry.h"

namespace Nome::PartialEdgeDS
{

class Edge;

class Vertex: public Entity
{
public:
    Entity *parent; // Can be a partial face, partial edge,
    // or partial vertex
    //TODO: Add Point Geometry
    EType type;
    Geometry *geometry;
    Vertex();
    ~Vertex();

    std::vector<Edge *> getEdges() const;
};

} /* namespace Nome::Scene::PartialEdgeDS */