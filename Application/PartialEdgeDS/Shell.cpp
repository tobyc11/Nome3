//
// Created by 殷子欣 on 2019/10/14.
//

#include "Shell.h"
#include "Region.h"
#include "PFace.h"

namespace Nome::PartialEdgeDS
{

Shell::~Shell()
{
    this->killShell();

    if (this->next != NULL)
    {
        delete this->next;
        this->next = NULL;
    }
}

Shell* Shell::killShell()
{
    if (this->pface != NULL)
    {
        delete this->pface;
        this->pface = NULL;
    }

    return this->next;
}

} /* namespace Nome::Scene::PartialEdgeDS */