#include "SceneGraph.h"
#include "ASTSceneAdapter.h"
#include "Entity.h"
#include <Parsing/ASTContext.h>

namespace Nome::Scene
{

void CSceneTreeNode::L2WTransformUpdate()
{
    if (!Parent)
    {
        L2WTransform.UpdateValue(Owner->Transform.GetValue(Matrix3x4::IDENTITY));
        return;
    }
    L2WTransform.UpdateValue(Parent->L2WTransform.GetValue(Matrix3x4::IDENTITY)
                             * Owner->Transform.GetValue(Matrix3x4::IDENTITY));
}

CEntity* CSceneTreeNode::GetEntity() const
{
    if (InstanceEntity)
        return InstanceEntity;
    return GetOwner()->GetEntity();
}

CSceneTreeNode* CSceneTreeNode::FindChildOfOwner(CSceneNode* owner) const
{
    for (auto* child : Children)
        if (child->GetOwner() == owner)
            return child;
    return nullptr;
}

CSceneTreeNode* CSceneTreeNode::FindChild(const std::string& name) const
{
    for (auto* child : Children)
        if (child->GetOwner()->GetName() == name)
            return child;
    return nullptr;
}

std::string CSceneTreeNode::GetPath() const
{
    if (!Parent)
        return std::string();

    if (Owner->IsGroup())
        return Parent->GetPath();

    std::string result;
    return Parent->GetPath() + "." + Owner->GetName();
}

CSceneTreeNode::CSceneTreeNode(CSceneNode* owner)
    : Owner(owner)
{
}

CSceneTreeNode* CSceneTreeNode::CreateTree(CSceneNode* dagNode)
{
    CSceneTreeNode* treeNode = new CSceneTreeNode(dagNode);
    for (CSceneNode* dagChild : dagNode->Children)
    {
        CSceneTreeNode* treeChild = CreateTree(dagChild);
        treeNode->Children.insert(treeChild);
        treeChild->Parent = treeNode;
    }

    // Tell the owner, and instantiate
    treeNode->Owner->TreeNodes.insert(treeNode);
    if (treeNode->Owner->Entity && treeNode->Owner->Entity->IsInstantiable())
        treeNode->InstanceEntity = treeNode->Owner->Entity->Instantiate(treeNode);
    return treeNode;
}

void CSceneTreeNode::RemoveTree()
{
    for (CSceneTreeNode* child : Children)
        child->RemoveTree();

    // Note: the tree node may still be referenced after deletion, thus we reset all relavant info
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
    OnTransformChange();
}

void CSceneNode::TransformMarkedDirty()
{
    for (CSceneTreeNode* treeNode : TreeNodes)
    {
        treeNode->MarkTreeL2WDirty();
    }
}

CSceneNode::CSceneNode(CScene* owningScene, std::string name, bool isRoot, bool isGroup)
    : Scene(owningScene)
    , Name(std::move(name))
    , bIsGroup(isGroup)
{
    if (isRoot)
    {
        auto* treeNode = new CSceneTreeNode(this);
        TreeNodes.insert(treeNode);
    }
}

CSceneNode::~CSceneNode() { }

bool CSceneNode::SetName(std::string newName)
{
    // Grab all children of one parent to test for name conflict
    if (!Parents.empty())
        for (const auto& childPtr : (*Parents.begin())->Children)
            if (childPtr->GetName() == newName)
                return false;
    Name = std::move(newName);
    return true;
}

void CSceneNode::AddParent(CSceneNode* newParent)
{
    // Don't do anything if it is already a parent, conceptually, this checks for multiedges
    if (Parents.find(newParent) != Parents.end())
        return;

    for (CSceneTreeNode* parentTreeNode : newParent->TreeNodes)
    {
        CSceneTreeNode* myTree = CSceneTreeNode::CreateTree(this);
        myTree->Parent = parentTreeNode;
        parentTreeNode->Children.insert(myTree);
    }

    Parents.insert(newParent);
    newParent->Children.insert(this);
}

void CSceneNode::RemoveParent(CSceneNode* parent)
{
    // Make sure parent is indeed a parent
    auto iter = Parents.find(parent);
    if (iter == Parents.end())
        return;

    // Undo the relationship
    Parents.erase(iter);
    auto iter2 = parent->Children.find(this);
    parent->Children.erase(iter2);

    // Destroy the associated sub-trees
    for (CSceneTreeNode* parentTreeNode : parent->TreeNodes)
    {
        CSceneTreeNode* myTree = parentTreeNode->FindChildOfOwner(this);
        myTree->RemoveTree();
    }
}

CSceneNode* CSceneNode::CreateChildNode(const std::string& name)
{
    auto* child = new CSceneNode(Scene, name);
    child->AddParent(this);
    return child;
}

CSceneNode* CSceneNode::FindChildNode(const std::string& name)
{
    for (const auto& child : Children)
        if (child->GetName() == name)
            return child.Get();
    return nullptr;
}

CSceneNode* CSceneNode::FindOrCreateChildNode(const std::string& name)
{
    auto* result = FindChildNode(name);
    if (!result)
        return CreateChildNode(name);
    return result;
}

size_t CSceneNode::CountTreeNodes() const { return TreeNodes.size(); }

const std::set<TAutoPtr<CSceneTreeNode>>& CSceneNode::GetTreeNodes() const { return TreeNodes; }

CEntity* CSceneNode::GetEntity() const { return Entity; }

void CSceneNode::SetEntity(CEntity* ent)
{
    if (ent == Entity)
        return;

    if (ent == nullptr)
    {
        if (Entity->IsInstantiable())
        {
            // Uninstantiate
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
                treeNode->InstanceEntity = Entity->Instantiate(treeNode);
            }
        }
    }
}

void CSceneNode::NotifySurfaceDirty() const
{
    for (CSceneTreeNode* treeNode : TreeNodes)
    {
        treeNode->SetEntityUpdated(true);
    }
}

void CSceneNode::SyncFromAST(AST::ACommand* cmd, CScene& scene)
{
    if (ASTSource)
        throw AST::CSemanticError("SceneNode already has an associated AST command", cmd);
    ASTSource = cmd;
    // This function is called for either a group or an instance;
    // for a group, there isn't much to be done
    if (cmd->GetCommand() == "group")
        return;

    TAutoPtr<CTransform> lastTransform;
    for (auto* namedArg : cmd->GetTransforms())
    {
        auto* transform = CASTSceneAdapter::ConvertASTTransform(namedArg);
        if (lastTransform)
            transform->Input.Connect(lastTransform->Output);
        lastTransform = transform;
    }
    if (lastTransform)
        Transform.Connect(lastTransform->Output);
}

void CSceneNode::SyncToAST(AST::CASTContext& ctx)
{
    throw "don't use";
}

AST::ACommand* CSceneNode::BuildASTCommand(AST::CASTContext& ctx) const
{
    auto* node = ctx.Make<AST::ACommand>(ctx.MakeToken("instance"), ctx.MakeToken("endinstance"));
    // There should be a way to map Type<->AST node easily?
    //   name:String         0:ident
    //   entity:EntityRef    1:ident
    //   transform:???       transform:[rotate:vec vec, translate: vec]
    //   surface:EntityRef   surface:ident
    auto* zero = ctx.MakeIdent(GetName());
    node->PushPositionalArgument(zero);
    auto* one = ctx.MakeIdent(GetEntity()->GetName());
    node->PushPositionalArgument(one);
    if (GetSurface())
    {
        auto* child = ctx.MakeIdent(GetSurface()->GetName());
        auto* surface = ctx.Make<AST::ANamedArgument>(ctx.MakeToken("surface"));
        surface->AddChild(child);
        node->AddNamedArgument(surface);
    }
    if (Transform.IsConnected())
        throw std::runtime_error("Can't write transformations into AST yet");
    std::cout << "Dumping newly generated instance command" << std::endl;
    std::cout << *node << std::endl;
    return node;
}

}
