//
// Created by 殷子欣 on 2019/10/14.
//

#pragma once
#include "Entity.h"
#include <vector>

namespace Nome::PartialEdgeDS
{

class Edge;
class Vertex;

class PVertex: public Entity
{
public:
    Edge *edge;
    PVertex *next; //TODO: Evaluate if we actually need this
    Vertex *vertex;

    std::vector<Edge *> getAndVisitEdges(Edge *edge) const;
};

} /* namespace Nome::Scene::PartialEdgeDS */