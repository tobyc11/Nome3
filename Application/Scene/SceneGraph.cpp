#include "SceneGraph.h"
#include "Entity.h"

namespace Nome::Scene
{

void CSceneTreeNode::L2WTransformUpdate()
{
	if (!Parent)
	{
		L2WTransform.UpdateValue(Owner->Transform.GetValue(Matrix3x4::IDENTITY));
		return;
	}
	L2WTransform.UpdateValue(
		Owner->Transform.GetValue(Matrix3x4::IDENTITY) * Parent->L2WTransform.GetValue(Matrix3x4::IDENTITY));
}

CSceneTreeNode::CSceneTreeNode(CSceneNode* owner) : Owner(owner)
{
}

CSceneTreeNode* CSceneTreeNode::CopyTree()
{
	std::set<CSceneTreeNode*> childrenOfCopy;
	for (CSceneTreeNode* child : Children)
	{
		//Copy child
		childrenOfCopy.insert(child->CopyTree());
	}

	CSceneTreeNode* myCopy = new CSceneTreeNode(Owner);
	myCopy->Children = childrenOfCopy;
	for (CSceneTreeNode* childOfCopy : myCopy->Children)
		childOfCopy->Parent = myCopy;
	//Tell the owner, and instantiate
	Owner->TreeNodes.insert(myCopy);
	if (Owner->Entity && Owner->Entity->IsInstantiable())
		myCopy->InstanceEntity = Owner->Entity->Instantiate();
		
	return myCopy;
}

void CSceneTreeNode::RemoveTree()
{
	for (CSceneTreeNode* child : Children)
		child->RemoveTree();

	//Note: the tree node may still be referenced after deletion, thus we reset all relavant info
	Parent = nullptr;
	Children.clear();
	auto iter = Owner->TreeNodes.find(this);
	Owner->TreeNodes.erase(iter);
	Owner = nullptr;
	InstanceEntity = nullptr;
}

void CSceneTreeNode::MarkTreeL2WDirty()
{
	for (CSceneTreeNode* child : Children)
		child->MarkTreeL2WDirty();

	L2WTransform.MarkDirty();
}

CSceneNode::CSceneNode(std::string name) : Name(std::move(name)) 
{
	auto* treeNode = new CSceneTreeNode(this);
	TreeNodes.insert(treeNode);
}

CSceneNode::~CSceneNode()
{
}

void CSceneNode::AddParent(CSceneNode* newParent)
{
    //Don't do anything if it is already a parent
    if (Parents.find(newParent) != Parents.end())
        return;

	for (CSceneTreeNode* parentTreeNode : newParent->TreeNodes)
	{
		CSceneTreeNode* myTreeCopy = (*TreeNodes.begin())->CopyTree();
		myTreeCopy->Parent = parentTreeNode;
		parentTreeNode->Children.insert(myTreeCopy);
	}

    Parents.insert(newParent);
    newParent->Children.insert(this);
}

void CSceneNode::RemoveParent(CSceneNode* parent)
{
	//Make sure parent is indeed a parent
    auto iter = Parents.find(parent);
    if (iter == Parents.end())
        return;

	//Undo the relationship
    Parents.erase(iter);
    auto iter2 = parent->Children.find(this);
    parent->Children.erase(iter2);

	//Destroy the associated sub-trees
	for (CSceneTreeNode* parentTreeNode : parent->TreeNodes)
	{
		CSceneTreeNode* myTree = parentTreeNode->FindChildOfOwner(this);
		myTree->RemoveTree();
	}
}

CSceneNode* CSceneNode::CreateChildNode(const std::string& name)
{
    auto* child = new CSceneNode(name);
    child->AddParent(this);
    return child;
}

size_t CSceneNode::CountTreeNodes() const
{
	return TreeNodes.size();
}

const std::set<TAutoPtr<CSceneTreeNode>>& CSceneNode::GetTreeNodes() const
{
	return TreeNodes;
}

CEntity* CSceneNode::GetEntity() const
{
	return Entity;
}

void CSceneNode::SetEntity(CEntity* ent)
{
	if (ent == Entity)
		return;

	if (ent == nullptr)
	{
		if (Entity->IsInstantiable())
		{
			//Uninstantiate
			for (CSceneTreeNode* treeNode : TreeNodes)
			{
				treeNode->InstanceEntity = nullptr;
			}
		}
		Entity = nullptr;
	}
	else
	{
		Entity = ent;
		if (Entity->IsInstantiable())
		{
			for (CSceneTreeNode* treeNode : TreeNodes)
			{
				treeNode->InstanceEntity = Entity->Instantiate();
			}
		}
	}
}

}
