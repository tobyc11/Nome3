//
// Created by 殷子欣 on 2019/10/14.
//

#include "Shell.h"
#include "Region.h"
#include "PFace.h"

namespace Nome::PartialEdgeDS
{

Shell::Shell(const Region *&region, const Shell *&next, const Shell *&prev, const pFace *&pFace) : \
          Entity(), region(region), next(next), prev(prev), pFace(pFace)
{
    if (prev != NULL) { prev->next = this; }
    if (next != NULL) { next->prev = this; }
}

Shell::~Shell()
{
    if (prev == NULL && next == NULL && region != NULL) {
        delete(region);
        return;
    }

    if (prev != NULL)
    {
        prev->next = next;
    } else {
        region->shell = next;
    }

    if (next != NULL) { next->prev = prev; }

}

void Shell::killChildren() {

    PFace *tempPFace = pFace, *deletePFace;
    while (tempPFace != NULL)
    {
        tempPFace->killChildren();
        deletePFace = tempPFace;
        tempPFace = tempPFace->next;
        delete(deletePFace);
    }
}

} /* namespace Nome::Scene::PartialEdgeDS */