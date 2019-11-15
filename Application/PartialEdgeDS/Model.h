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
    Model *prev;
    Region *region;
    u_int64_t modelUID;

    Model();
    ~Model();

    Model* killModel();
    void mergeModel(Model *bModel);
    void changeKeysInModel(std::map<std::pair<EGType, u_int64_t>, u_int64_t> *keyMap);
};

}