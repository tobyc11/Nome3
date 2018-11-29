#pragma once

#include "Scene/Entity.h"
#include "HEMesh.h"

namespace Nome
{

using tc::TAutoPtr;

class CHEMeshRenderEntity : public Scene::CEntity
{
public:
    explicit CHEMeshRenderEntity(CHEMesh* mesh) : Mesh(mesh)
    {
    }

    void Draw() const override;

private:
    TAutoPtr<CHEMesh> Mesh;
};

}
