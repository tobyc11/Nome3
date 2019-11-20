//
// Created by 殷子欣 on 2019/10/14.
//

#include "Region.h"
#include "Face.h"
#include "Model.h"
#include "PFace.h"
#include "Shell.h"

namespace Nome::PartialEdgeDS
{

Region::~Region()
{
    if (prev == NULL && next == NULL && model != NULL) {
        delete(model);
        return;
    }

    if (prev != NULL)
    {
        prev->next = next;
    } else {
        model->region = next;
    }

    if (next != NULL) { next->prev = prev; }

}

void Region::killChildren() {

    Shell *tempShell = shell, *deleteShell;
    while (tempShell != NULL)
    {
        tempShell->killChildren();
        tempShell->region = NULL;
        deleteShell = tempShell;
        tempShell = tempShell->next;
        delete(deleteShell);
    }
}

std::vector<Face *> Region::getFaces() const
{
    std::vector<Face *> faces;
    Shell *tempShell = shell;

    while (tempShell != NULL)
    {
        PFace *tempPFaceP = tempShell->pface;
        while (tempPFaceP != NULL)
        {
            if (tempPFaceP->type == EType::FACE && tempPFaceP->child != NULL) { faces.push_back((Face *)tempPFaceP->child); }
            tempPFaceP = tempPFaceP->next;
        }
        tempShell = tempShell->next;
    }
    return faces;
}

} /* namespace Nome::Scene::PartialEdgeDS */