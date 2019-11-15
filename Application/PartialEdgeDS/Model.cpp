//
// Created by 殷子欣 on 2019/10/14.
//

#include "Model.h"
#include "Region.h"

namespace Nome::PartialEdgeDS
{

Model::~Model()
{
    this->killModel();

    if (this->next != NULL)
    {
        delete this->next;
        this->next = NULL;
    }
}

Model* Model::killModel()
{
    if (this->region != NULL)
    {
        delete this->region;
        this->region = NULL;
    }

    return this->next;
}

bool cutFaceByTwoVertexes(const std::string &vertexID1, const std::string &vertexID2)
{
    return true;
}

std::vector<Region *> getRegions() const {
    std::vector<Region *> regions;
    Region *tempRegion = region;

    while (tempRegion != NULL)
    {
        regions.push_back(tempRegion);
        tempRegion = tempRegion->next;
    }
    return regions;
}

} /* namespace Nome::Scene::PartialEdgeDS */