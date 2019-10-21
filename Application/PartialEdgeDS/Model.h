//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"

namespace Nome::PartialEdgeDS {

class Region;

class Model: public Entity
{
public:
    Model *next;
    Region *region;

    Model();
    ~Model();

    Model* killModel();
};

}