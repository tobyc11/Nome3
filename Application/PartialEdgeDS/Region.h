//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include "Model.h"
#include "Shell.h"

namespace Nome::PartialEdgeDS {

class Region: public Entity
{
public:
    Model *model;
    Region *next;
    Shell *shell;
};

}