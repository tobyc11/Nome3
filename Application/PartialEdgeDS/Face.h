//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include <vector>

namespace Nome::PartialEdgeDS {

class PFace;
class Loop;

class Face: public Entity
{
public:
    PFace *pface;
    Loop *loop;
    //TODO: Add Surface Geometry

    Face();
    ~Face();

    std::vector<Edge *> getEdges() const;

    Face* killFace();
};


}