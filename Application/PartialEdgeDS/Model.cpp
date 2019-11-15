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

} /* namespace Nome::Scene::PartialEdgeDS */