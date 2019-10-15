//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"

namespace Nome::PartialEdgeDS {

class Model;
class Shell;

class Region: public Entity
{
public:
    Model *model;
    Region *next;
    Shell *shell;
};

}