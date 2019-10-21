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
    PEdge *pedge;

    Loop();
    ~Loop();

    Loop* killLoop();
};

} /* namespace Nome::Scene::PartialEdgeDS */

