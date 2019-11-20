//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include "Geometry.h"
#include <vector>

namespace Nome::PartialEdgeDS {

class PFace;
class Loop;

class Face: public Entity
{
public:
    PFace *pFace;
    Loop *loop;
    //TODO: Add Surface Geometry
    Geometry *geometry;
    Face();
    ~Face();

    std::vector<Edge *> getEdges() const;
};


}