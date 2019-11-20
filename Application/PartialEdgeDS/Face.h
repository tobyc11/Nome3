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
    Face(const PFace *&pFace, const Loop *&loop, const Geometry *&geometry);
    ~Face();

    std::vector<Edge *> getEdges() const;
};


}