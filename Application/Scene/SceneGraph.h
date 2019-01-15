#pragma once
#include "Transforms.h"

#include <map>
#include <set>
#include <string>
#include <utility>

namespace Nome::Scene
{

using tc::TAutoPtr;

//Forward declaration
class CSceneNode;
class CEntity;

class CSceneTreeNode : public Flow::CFlowNode
{
	//The matrix that takes you from local coord to world
	DEFINE_OUTPUT_WITH_UPDATE(Matrix3x4, L2WTransform);

public:
	//Public APIs
	CSceneNode* GetOwner() const { return Owner; }
	bool IsValid() const { return Owner; }

	//Note: linear time is prob too slow
	CSceneTreeNode* FindChildOfOwner(CSceneNode* owner) const
	{
		for (auto* child : Children)
			if (child->GetOwner() == owner)
				return child;
		return nullptr;
	}

private:
	//Only CSceneNode manages the tree nodes
	friend class CSceneNode;

	explicit CSceneTreeNode(CSceneNode* owner);

	//Copy the entire tree, the copy of this node will not have a parent
	CSceneTreeNode* CopyTree();
	void RemoveTree();
	void MarkTreeL2WDirty();

	//Private fields, accessible to CSceneNode though
    CSceneNode* Owner;
    CSceneTreeNode* Parent = nullptr;
    std::set<CSceneTreeNode*> Children;

	//This is non-null if the entity is instantiable
	TAutoPtr<CEntity> InstanceEntity;
};

class CSceneNode : public Flow::CFlowNode
{
	DEFINE_INPUT(Matrix3x4, Transform)
	{
	}

public:
	explicit CSceneNode(std::string name);
    ~CSceneNode() override;

	//Hierarchy management
    void AddParent(CSceneNode* newParent);
    void RemoveParent(CSceneNode* parent);
	CSceneNode* CreateChildNode(const std::string& name);

	//Returns the number of associated tree nodes, i.e. the number of ways from the root to this graph node
    size_t CountTreeNodes() const;
    const std::set<TAutoPtr<CSceneTreeNode>>& GetTreeNodes() const;

	//Instance/Entity related
	CEntity* GetEntity() const;
	void SetEntity(CEntity* ent);

private:
    std::string Name;

	friend class CSceneTreeNode;
    //Parents and associated tree nodes organized by parent
    std::set<CSceneNode*> Parents;
    std::set<TAutoPtr<CSceneNode>> Children;
	std::set<TAutoPtr<CSceneTreeNode>> TreeNodes;

	//Associated entity, aka the generator instantiated
	TAutoPtr<CEntity> Entity;
};

}
