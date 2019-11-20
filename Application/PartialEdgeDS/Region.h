//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include <vector>

namespace Nome::PartialEdgeDS {

class Model;
class Shell;
class Face;

class Region: public Entity
{
public:
    Model *model;
    Region *next;
    Region *prev;
    Shell *shell;

    Region();
    ~Region();

    std::vector<Face *> getFaces() const;
};

}