//
// Created by 殷子欣 on 2019/10/14.
//

#pragma once
#include "Entity.h"

namespace Nome::PartialEdgeDS
{

class Face;
class PEdge;

class Loop: public Entity
{
public:
    Face *face;
    Loop *next;
    Loop *prev;
    PEdge *pEdge;

    Loop(const Face *&face, const Loop *&next, const Loop *&prev, const PEdge *&pEdge);
    ~Loop();
};

} /* namespace Nome::Scene::PartialEdgeDS */

