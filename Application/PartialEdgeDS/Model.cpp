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

    //Kill GeometryMapper[Model_UID, _* ]

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
    /**
     * TODO: Implement Merge Models for merging A and B
     *  - First, Merge HashTables, get list of new B ids
     *  - Apply changes to B for ids at Faces, Edges, Vertices
     *  - Append Region(s) from B to A
     *  - Null out *region
     *  - Kill B -> return pointer to next guy after B
     *  - Stitch to previous
     *  - Free B
    */

void Model::mergeModel(Model *bModel)
{
    //First, Merge HashTables, get list of new B ids
    auto keysToChangeInB = Entity::mapper.mergeModels(uid, bModel->uid);

    //Apply changes to B for ids at Faces, Edges, Vertices
    changeKeysInModel(keysToChangeInB);

    //Append Region(s) from B to A
    if (region == NULL)
    {
        region = bModel->region;
    }
    else
    {
        Region* current = region;
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = bModel->region;
    }

    //Null out *region
    bModel->region = NULL;

    //Kill B, then stitch to previous
    Model *beforeB = bModel->prev;
    Model *afterB = bModel->killModel();
    beforeB->next = afterB;

    //Delete B
    bModel->next = NULL;    //Prevent it from deleting the rest of the models
    delete bModel;
}

void Model::changeKeysInModel(std::map<std::pair<EGType, u_int64_t>, u_int64_t> *keyMap)
{

}

} /* namespace Nome::Scene::PartialEdgeDS */
