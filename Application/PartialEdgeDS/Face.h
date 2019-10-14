//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include "PFace.h"

namespace Nome::PartialEdgeDS {

class Face: public Entity
{
public:
    PFace *pface;
    Loop *loop;
    //TODO: Add Surface Geometry
};


}