//
// Created by peter on 10/14/19.
//

#pragma once
#include <string>
#include <map>
#include "Manager.h"

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
    static Manager mapper;

    virtual ~Entity() { };
    //TODO: ADD Init
    Entity(u_int64_t uid) : uid(std::move(uid)) { }
    bool isVisit = false;
private:
    u_int64_t uid;
};

}
