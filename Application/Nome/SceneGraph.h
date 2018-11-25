#pragma once

#include "Transforms.h"

#include <set>

namespace Nome::Scene
{

using tc::TAutoPtr;

class CSceneTreeNode : public Flow::CFlowNode
{
public:

private:
};

class CSceneNode : public Flow::CFlowNode
{
public:
    DEFINE_INPUT(Matrix3x4, Transform)
    {
        TransformDirty.MarkDirty();
    }

    //Dummy output used to notify associated scene tree nodes
    //  The bool value itself is never used
    DEFINE_OUTPUT_WITH_UPDATE(void, TransformDirty)
    {
        TransformDirty.UnmarkDirty();
    }

private:
    std::set<CSceneNode*> Parents;
    std::set<TAutoPtr<CSceneNode>> Children;

    std::set<TAutoPtr<CSceneTreeNode>> AssocTreeNodes;
};

}
