#include "SceneGraph.h"
#include "Entity.h"

namespace Nome::Scene
{

void CSceneTreeNode::AddChild(CSceneTreeNode* node)
{
    node->SetParent(this);
}

void CSceneTreeNode::SetParent(CSceneTreeNode* parNode)
{
    Parent = parNode;
}

void CSceneTreeNode::MarkTransformDirty()
{
    L2WDirty = true;
}

const Matrix3x4& CSceneTreeNode::GetL2W() const
{
    if (L2WDirty)
    {
        if (Parent)
        {
            L2W = Owner->Transform.GetValue(Matrix3x4::IDENTITY) * Parent->GetL2W();
        }
        else
        {
            L2W = Owner->Transform.GetValue(Matrix3x4::IDENTITY);
        }
        L2WDirty = false;
    }

    return L2W;
}

CSceneNode::CSceneNode(std::string name, bool isRoot) : Name(std::move(name)), bIsRoot(isRoot)
{
}

void CSceneNode::AddParent(CSceneNode* newParent)
{
    //Don't do anything if it is already a parent
    if (Parents.find(newParent) != Parents.end())
        return;

    AssocTreeNodes.insert(std::make_pair(newParent, std::set<TAutoPtr<CSceneTreeNode>>()));
    for (auto& pair : newParent->AssocTreeNodes)
    {
        for (auto& parTreeNode : pair.second)
        {
            auto* myTreeNode = CreateTreeNode();
            parTreeNode->AddChild(myTreeNode);

            AssocTreeNodes[newParent].insert(myTreeNode);
        }
    }

    Parents.insert(newParent);
}

void CSceneNode::RemoveParent(CSceneNode* parent)
{
    auto iter = Parents.find(parent);
    if (iter == Parents.end())
        return;

    Parents.erase(iter);

    for (auto& treeNode : AssocTreeNodes[parent])
    {
        treeNode->SetParent(nullptr);
    }
    AssocTreeNodes.erase(parent);
}

const TAutoPtr<CEntity>& CSceneNode::GetEntity() const
{
    return Entity;
}

void CSceneNode::SetEntity(const TAutoPtr<CEntity>& value)
{
    Entity = value;
}

CSceneTreeNode* CSceneNode::CreateTreeNode()
{
    return new CSceneTreeNode(this);
}

}
