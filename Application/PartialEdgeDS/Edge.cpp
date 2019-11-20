//
// Created by 殷子欣 on 2019/10/14.
//

#include "Edge.h"
#include "PVertex.h"

namespace Nome::PartialEdgeDS
{
Edge::Edge(const PVertex *&pVertices[2], const Entity *&parent, const EType &type) : \
           Entity(), pVertices(pVertices), parent(parent), type(type)
{

}

Edge::~Edge()
{
    if (geometry != NULL) { delete(geometry); }
    if (parent != NULL) { delete(parent); }
}

void Edge::killChildren() {

    for (int i = 0; i < 2; i++)
    {
        if (pVertices[i] != NULL)
        {
            pVertices[i]->killChildren();
            pVertices[i]->parent = NULL;
            delete(pVertices[i]);
        }
    }
}

} /* namespace Nome::Scene::PartialEdgeDS */