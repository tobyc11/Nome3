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

    Loop();
    ~Loop();
};

} /* namespace Nome::Scene::PartialEdgeDS */

