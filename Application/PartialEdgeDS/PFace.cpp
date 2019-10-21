//
// Created by 殷子欣 on 2019/10/14.
//

#include "PFace.h"
#include "Shell.h"

namespace Nome::PartialEdgeDS
{

PFace::~PFace()
{
    this->killPFace();

    if (this->next != NULL)
    {
        delete this->next;
        this->next = NULL;
    }
}

PFace* PFace::killPFace()
{
    if (this->child != NULL)
    {
        delete this->child;
        this->child = NULL;
    }

    return this->next;
}

} /* namespace Nome::Scene::PartialEdgeDS */