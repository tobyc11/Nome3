//
// Created by 殷子欣 on 2019/10/14.
//

#include "PFace.h"
#include "Shell.h"
#include "Face.h"
#include "Edge.h"
#include "Vertex.h"

namespace Nome::PartialEdgeDS
{

PFace::PFace(const Shell *&shell, const PFace *&next, const PFace *&prev, const Entity &*child, const EType &type) \
            : shell(shell), next(next), prev(prev), child(child), type(type)
{
  if (prev != NULL) { prev->next = this; }
  if (next != NULL) { next->prev = this; }
}

PFace::~PFace()
{
    if (prev == NULL && next == NULL && shell != NULL) {
        delete(shell);
        return;
    }

    if (prev != NULL) {
        prev->next = next;
    } else {
        shell->pFace = next;
    }

    if (next != NULL) { next->prev = prev; }

}

void PFace::killChildren() {

    if (child != NULL)
        child->killChildren();
        switch (type) {
        case EType::FACE :
            ((Face *)child)->pFace = NULL;
            break;
        case EType::EDGE :
            ((Edge *)child)->parent = NULL;
            break;
        case EType::VERTEX :
            ((Vertex *)child)->parent = NULL;
            break;
        case default:
            break;
        }
        delete(child);
    }
}

} /* namespace Nome::Scene::PartialEdgeDS */