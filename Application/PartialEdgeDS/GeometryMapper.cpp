//
// Created by 殷子欣 on 2019/10/25.
//

#include "GeometryMapper.h"

namespace Nome::PartialEdgeDS

{

GeometryMapper::GeometryMapper() {

}

bool GeometryMapper::copyModel(const std::string &model_uid){
    return true;
}

bool GeometryMapper::addGeometry(const std::string &model_uid,
                                 const EGType &type,
                                 const std::string &geometry_uid,
                                 const Geometry &geometry) {

    if (map[model_uid].empty()) { map.insert(std::pair(model_uid,
                                            std::map<EGType,std::map<std::string, Geometry>>()));
    }
    if (map[model_uid][type].empty()) { map[model_uid].insert(std::pair(type,
                                                             std::map<std::string, Geopmetry>()));
    }
    if (map[model_uid][type].find(geometry_uid) != map[model_uid][type].end()) {
        std::cout <<"GeometryMapper::addGeometry(const std::string &" << model_uid << \
        ", const EGType &" << type << ", const std::string &" << geometry_uid << "does exits!!";
        return false;
    }

    map[model_uid][type][geometry_uid] = geopmetry;
    return true;
}

bool killModel(const std::string &model_uid) {
    std::map<char,int>::iterator it = map.find(model_uid);
    if (it != map.end()) {
        map.erase (it);
        return true;
    }
    return false;
}

bool copyModel(const std::string &model_uid, const std::string &new_model_uid) {
    std::map<char,int>::iterator it = map.find(model_uid);
    if (it == map.end()) {
        return false
    }
    map.insert(std::pair(new_model_uid, map[model_uid]);
    return true;
}

}