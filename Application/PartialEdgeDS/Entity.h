//
// Created by peter on 10/14/19.
//

#pragma once
#include <string>
#include <map>
#include "GeometryMapper.h"

namespace Nome::PartialEdgeDS {

enum class EType {
    FACE,
    EDGE,
    VERTEX,
    PFACE,
    PEDGE,
    PVERTEX
};


class Entity
{
public:
    static GeometryMapper mapper;

    virtual ~Entity() { };
    //TODO: ADD Init
    Entity(std::string name) : Name(std::move(name)) { }
    bool isVisit = false;
private:
    std::string Name;
};

}
