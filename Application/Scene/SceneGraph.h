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
public:
    const Matrix3x4& GetL2W() const;

    CSceneNode* GetOwner() const
    {
        return Owner;
    }

    template <typename T>
    void ForEach(T fn)
    {
        fn(this);
        for (auto child : Children)
        {
            child->ForEach(fn);
        }
    }

private:
    //Ensure only CSceneNode can create instances of tree node
    friend class CSceneNode;

    explicit CSceneTreeNode(CSceneNode* owner) : Owner(owner)
    {
    }

    void AddChild(CSceneTreeNode* node);

    void SetParent(CSceneTreeNode* parNode);

    //Called by the graph node when its transform is dirty
    void MarkTransformDirty();

    //Private fields
    CSceneNode* Owner;
    CSceneTreeNode* Parent = nullptr;
    std::set<CSceneTreeNode*> Children;

    ///Local to world transform
    mutable Matrix3x4 L2W;
    mutable bool L2WDirty = true;
};

class CSceneNode : public Flow::CFlowNode
{
public:
    explicit CSceneNode(std::string name, bool isRoot = false);

    ~CSceneNode() override;

    bool IsInScene() const
    {
        return bIsRoot || !Parents.empty();
    }

    void AddParent(CSceneNode* newParent);
    void RemoveParent(CSceneNode* parent);
    CSceneNode* CreateChildNode(const std::string& name = "unnamed");

    const TAutoPtr<CEntity>& GetEntity() const;
    void SetEntity(const TAutoPtr<CEntity>& value);

    size_t CountTreeNodes() const;

    std::set<TAutoPtr<CSceneTreeNode>> GetTreeNodes() const;

    Matrix3x4 GetTransform() const;
    void SetDefaultTransform(const Matrix3x4& val)
    {
        DefaultTransform = val;
    }

    ///Use GetTransform instead of accessing this directly
    DEFINE_INPUT(Matrix3x4, Transform)
    {
        for (auto& pair : AssocTreeNodes)
        {
            for (auto& treeNode : pair.second)
            {
                treeNode->MarkTransformDirty();
            }
        }
    }

private:
    CSceneTreeNode* CreateTreeNode();

private:
    std::string Name;

    bool bIsRoot = false;

    //Parents and associated tree nodes organized by parent
    std::set<CSceneNode*> Parents;
    std::map<CSceneNode*, std::set<TAutoPtr<CSceneTreeNode>>> AssocTreeNodes;

    //Owns children
    std::set<TAutoPtr<CSceneNode>> Children;

    //Associated entity
    TAutoPtr<CEntity> Entity;

    ///The transform used when the input is disconnected
    Matrix3x4 DefaultTransform;
};

}
