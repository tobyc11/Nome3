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
    if (Parent == parNode)
        return;

    if (Parent)
    {
        auto iter = Parent->Children.find(this);
        Parent->Children.erase(iter);
    }

    Parent = parNode;
    Parent->Children.insert(this);
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
            L2W = Owner->GetTransform() * Parent->GetL2W();
        }
        else
        {
            L2W = Owner->GetTransform();
        }
        L2WDirty = false;
    }

    return L2W;
}

CSceneNode::CSceneNode(std::string name, bool isRoot) : Name(std::move(name)), bIsRoot(isRoot)
{
    if (isRoot)
    {
        //Create root tree node
        auto* myTreeNode = CreateTreeNode();
        AssocTreeNodes[nullptr] = std::set<TAutoPtr<CSceneTreeNode>>();
        AssocTreeNodes[nullptr].insert(myTreeNode);
    }
}

CSceneNode::~CSceneNode()
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
    newParent->Children.insert(this);
}

void CSceneNode::RemoveParent(CSceneNode* parent)
{
    auto iter = Parents.find(parent);
    if (iter == Parents.end())
        return;

    Parents.erase(iter);

    auto iter2 = parent->Children.find(this);
    parent->Children.erase(iter2);

    for (auto& treeNode : AssocTreeNodes[parent])
    {
        treeNode->SetParent(nullptr);
    }
    AssocTreeNodes.erase(parent);
}

CSceneNode* CSceneNode::CreateChildNode(const std::string& name)
{
    auto* child = new CSceneNode(name);
    child->AddParent(this);
    return child;
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

size_t CSceneNode::CountTreeNodes() const
{
    size_t count = 0;
    for (auto& pair : AssocTreeNodes)
        count += pair.second.size();
    return count;
}

std::set<TAutoPtr<CSceneTreeNode>> CSceneNode::GetTreeNodes() const
{
    std::set<TAutoPtr<CSceneTreeNode>> result;
    for (auto& pair : AssocTreeNodes)
        result.insert(pair.second.begin(), pair.second.end());
    return result;
}

Matrix3x4 CSceneNode::GetTransform() const
{
    return Transform.GetValue(DefaultTransform);
}

}
