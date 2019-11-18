//
// Created by 殷子欣 on 2019/10/25.
//

#include "Manager.h"

namespace Nome::PartialEdgeDS

{

Manager::Manager() {

}

void Manager::killModel(u_int64_t modelUID)
{
    auto it = map.find(modelUID);
    if (it != map.end())
    {
        map.erase(it);
    }
    else
    {
        // Throw an error and/or warning
    }
    
}

void Manager::addGeometry(u_int64_t modelUID,
                                 EGType type,
                                 u_int64_t geometryUID,
                                 Geometry *geometry)
{
    if (map[modelUID].empty())
    {
        map.insert(std::pair(modelUID, std::map<std::pair<EGType, u_int64_t>, Geometry*>()));
    }

    if (map[modelUID].find(geometry_key(type, geometryUID)) == map[modelUID].end())
    {
        map[modelUID][geometry_key(type, geometryUID)] = geometry;
    }
    else
    {
        //Throw an error and/or warning
    }

}

u_int64_t Manager::copyModel(u_int64_t modelUID)
{
    if (map.find(modelUID) != map.end())
    {
        u_int64_t new_modelUID = idGenerator.newUID();
        map.insert(std::pair(new_modelUID, std::map<std::pair<EGType, u_int64_t>, Geometry*>()));
        auto model_iter = map[modelUID].begin();

        while (model_iter != map[modelUID].end())
        {
            auto key = model_iter->first;
            map[new_modelUID][key] = new Geometry(*(model_iter->second));
            ++model_iter;
        }
    }
    else
    {
        // Throw an error and/or warning
    }
    
}

std::map<std::pair<EGType, u_int64_t>, u_int64_t> *Manager::mergeModels(u_int64_t aModelUID,
                                                                               u_int64_t bModelUID)
{
    if (map.find(aModelUID) == map.end() || map.find(bModelUID) == map.end())
    {
        // Throw an error and/or warning
    }

    auto aModel = map[aModelUID];
    auto bModel = map[bModelUID];

    // Create a new map of values to change in model B after the merge (this is what will be returned)
    auto keysToChangeInB = new std::map<std::pair<EGType, u_int64_t>, u_int64_t>();

    // Begin copying over bModel to aModel
    auto bModel_iter = bModel.begin();
    while (bModel_iter != bModel.end())
    {
        auto bKey = bModel_iter->first;
        auto bValue = bModel_iter->second;
        if (aModel.find(bKey) == aModel.end())
        {
            aModel[bKey] = bValue;
        }
        else
        {
            auto newKey = geometry_key(bKey.first, idGenerator.newUID());
            aModel[newKey] = bValue;
            (*keysToChangeInB)[bKey] = newKey.second;
        }
        
        ++bModel_iter;
    }

    killModel(bModelUID);

    return keysToChangeInB;
}

std::pair<EGType, u_int64_t> Manager::geometry_key(EGType type, u_int64_t geometryUID)
{
    return std::pair(type, geometryUID);
}

u_int64_t Manager::UIDGenerator::newUID()
{
    int64_t uid = counter;
    counter++;
    return uid;
}

}