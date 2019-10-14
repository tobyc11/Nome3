//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include "Region.h"
#include "PFace.h"

namespace Nome::PartialEdgeDS {

class Shell: public Entity
{
public:
    Region *region;
    Shell *next;
    PFace *pface;
};

}