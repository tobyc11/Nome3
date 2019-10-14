//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include "Region.h"

namespace Nome::PartialEdgeDS {

class Model: public Entity
{
public:
    Model *next;
    Region *region;
};

}