#include "Scene.h"
#include "SweepControlPoint.h"
#include "InteractivePoint.h"
#include "Mesh.h"
#include "MeshMerger.h"
#include <StringUtils.h>

namespace Nome::Scene
{

CScene::CScene()
{
    RootNode = new CSceneNode(this, "root", true);
    PickingMgr = new CPickingManager(this);
}

TAutoPtr<CSceneTreeNode> CScene::GetRootTreeNode() const
{
    const auto& rootTreeNodes = RootNode->GetTreeNodes();
    assert(rootTreeNodes.size() == 1);
    return *rootTreeNodes.begin();
}

void CScene::AddEntity(TAutoPtr<CEntity> entity)
{
    auto iter = EntityLibrary.find(entity->GetName());
    if (iter != EntityLibrary.end())
        EntityLibrary.erase(iter);
    EntityLibrary.insert(std::make_pair(entity->GetName(), std::move(entity)));
}

void CScene::RemoveEntity(const std::string& name, bool bAlsoRemoveChildren)
{
    auto iter = EntityLibrary.find(name);
    if (iter != EntityLibrary.end())
    {
        if (bAlsoRemoveChildren)
            while (tc::FStringUtils::StartsWith(iter->first, name))
                EntityLibrary.erase(iter++);
        else
            EntityLibrary.erase(iter);
    }
}

void CScene::DoneVisitingMesh(std::string meshName)
{
    orderedMeshNames.push_front(meshName); // first element is always the most recently added mesh's name
}

bool CScene::RenameEntity(const std::string& oldName, const std::string& newName)
{
    // New name already exists
    if (EntityLibrary.find(newName) != EntityLibrary.end())
        return false;

    // If entity with oldName not found
    auto iter = EntityLibrary.find(oldName);
    if (iter == EntityLibrary.end())
        return false;

    while (tc::FStringUtils::StartsWith(iter->first, oldName))
    {
        auto nh = EntityLibrary.extract(iter++);
        nh.key() = newName + nh.key().substr(oldName.length());
        nh.mapped()->SetName(nh.key());
        EntityLibrary.insert(std::move(nh));
    }
    return true;
}

TAutoPtr<CEntity> CScene::FindEntity(const std::string& name) const
{
    auto iter = EntityLibrary.find(name);
    if (iter != EntityLibrary.end())
        return iter->second;
    return nullptr;
}

TAutoPtr<CSceneNode> CScene::CreateGroup(const std::string& name)
{
    if (Groups.find(name) != Groups.end())
        return {};

    auto* node = new CSceneNode(this, name, false, true);
    Groups[name] = node;
    return node;
}

TAutoPtr<CSceneNode> CScene::CreateMerge(const std::string& name)
{
    if (Merges.find(name) != Merges.end())
        return {};

    auto* node = new CSceneNode(this, name, false, true, true);
    Merges[name] = node;
    return node;
}

TAutoPtr<CSceneNode> CScene::FindGroup(const std::string& name) const
{
    auto iter = Groups.find(name);
    if (iter != Groups.end())
        return iter->second;
    return nullptr;
}

Flow::TOutput<CVertexInfo*>* CScene::FindPointOutput(const std::string& id) const
{
    /* Find the point that is being referenced using the following naming convetions:
    Case #1. .pointID = global point
    Case #2. pointID  = mesh point in the mesh command we're currently constructing
    Case #3. meshID.pointID =  mesh point in the mesh command w/ the ID "meshName". If the ID refers
    to the current mesh, it works but is redudant w/ Case #2. 
    Case #4. instanceID.pointID = point in a mesh/polyline-related instance with the ID "instanceID".
    */
   

    // Handle case #2. This handles the case where "pointName" should refer to a point in a mesh we're currently parsing
    // If the id does not have a period, it is not a global point nor a different mesh's point
    if (id.find('.') == std::string::npos)
    { 
        for (const auto& NameEntity : EntityLibrary)
        {
            // Recall all entities were named EntityNamePrefix + cmd->GetName() in ASTSceneAdapter.cpp
            // e.g., most shape generators (including global points/faces) are simply named after their
            // ID e.g, points defined within a mesh construct are named meshID.pointID e.g., faces
            // defined within a mesh construct are named meshID.faceID
            std::string idWithPeriod = "." + id;
            auto entity = NameEntity.second;
            if (entity.Get()->GetMetaObject().ClassName() == "CPoint")
            {
                std::string pointName = NameEntity.first; // entity is guaranteed to be a point entity
                // if pointName has ".id" suffix (this implies the pointName is located in a mesh)
                if (pointName.find(idWithPeriod) != std::string::npos)
                {
                    // Extract from meshName from meshName.pointName
                    std::string meshName = pointName.substr(0, pointName.find(idWithPeriod));

                    // if the mesh is not previously defined, then the pointName is defined in a mesh
                    // we're currently visiting (haven't visited all its subcommands yet)! We need to treat
                    // this special case because the id parameter is currently just "ID", when we really
                    // want the point named "meshID.pointID". side note: if the mesh is previosly
                    // defined, then the below BFS will find the correct point for us
                    if (std::find(orderedMeshNames.begin(), orderedMeshNames.end(), meshName)
                        == orderedMeshNames.end())
                    {
                        // we've now figured out the id argument is actually referring to meshID.pointID
                        // for a mesh we haven't finished parsing yet.
                        return FindPointOutput(pointName);
                    }
                }
            }
        }
    }

    size_t charsToIgnore = 0;
    if (id[0] == '.')
        charsToIgnore = 1;

    // Handle case #1, #2, #3
    auto iter = EntityLibrary.find(id.substr(charsToIgnore, id.size()));

    if (iter != EntityLibrary.end())
    {
        TAutoPtr<CEntity> ent = iter->second;
        TAutoPtr<CPoint> point = dynamic_cast<CPoint*>(ent.Get());
        if (point)
        {
            return &point->Point;
        }
        TAutoPtr<CSweepControlPoint> controlPoint = dynamic_cast<CSweepControlPoint*>(ent.Get());
        if (controlPoint)
        {
            return &controlPoint->SweepControlPoint;
        }
    }

    // Handle case #4 (also the specific case #4 where the pointID refers to a mesh point)
    // Descend down the scene tree starting from the root
    CSceneTreeNode* currNode = *RootNode->GetTreeNodes().begin();
    while (true)
    {
 
        CSceneTreeNode* nextNode = nullptr;
        size_t nextDot = id.find('.', charsToIgnore);
        if (nextDot != std::string::npos)
        {
            // nextSeg = instance node name
            std::string nextSeg = id.substr(charsToIgnore, nextDot - charsToIgnore);
            nextNode = currNode->FindChild(nextSeg); 
        }
        if (!nextNode)
        {
            if (auto* meshInstance = dynamic_cast<CMeshInstance*>(currNode->GetInstanceEntity()))
            {
                std::string idTurnedVertName = id;
                std::replace(idTurnedVertName.begin(), idTurnedVertName.end(), '.', '_');
                // Handle special case #4 that references a mesh point. e.g., instanceID.pointID where pointID should refer to meshID.pointID entity.
                for (const auto& NameEntity : EntityLibrary)
                {
                    auto name = NameEntity.first;
                    auto entity = NameEntity.second;
                    // Check if adding this entity's name creates a valid name (e.g., mesh2.meshPointID). If so, we find the point correspoding to this combined name
                    auto potentialMeshPointName = name + "." + id.substr(charsToIgnore);
                    if (EntityLibrary.count(potentialMeshPointName) != 0)
                    {
                        // Handle case #4 that uses a mesh point in the mesh command
                        if (NameEntity.first.find(id.substr(charsToIgnore)))
                        {
                            TAutoPtr<CEntity> ent = EntityLibrary.at(potentialMeshPointName);
                            TAutoPtr<CPoint> point = dynamic_cast<CPoint*>(ent.Get());
                            if (point)
                            {
                                return &point->Point;
                            }
                        }
                    }
        
                }
                auto* point = meshInstance->CreateVertexSelector(id.substr(charsToIgnore), idTurnedVertName);
                if (point)
                {
                    return &point->Point;
                }
                else
                {
                    return nullptr;
                }
            }
            else
            {
                return nullptr;
            }
        }
        currNode = nextNode;
        // If the current node has only 1 child, it might be a group instance
        if (currNode->GetChildren().size() == 1)
        {
            CSceneTreeNode* onlyChild = *currNode->GetChildren().begin();
            if (onlyChild->GetOwner()->IsGroup())
                currNode = onlyChild;
        }
        charsToIgnore = nextDot + 1;
    }
    return nullptr;
}

std::pair<CSceneTreeNode*, std::string> CScene::WalkPath(const std::string& path) const
{
    auto pathComps = tc::FStringUtils::Split(path, ".");
    auto iter = pathComps.begin();
    if (iter->empty())
        ++iter;
    CSceneTreeNode* currNode = *RootNode->GetTreeNodes().begin();
    for (; iter != pathComps.end(); ++iter)
    {
        // If the current node has only 1 child, it might be a group instance
        if (currNode->GetChildren().size() == 1)
        {
            CSceneTreeNode* onlyChild = *currNode->GetChildren().begin();
            if (onlyChild->GetOwner()->IsGroup())
                currNode = onlyChild;
        }

        CSceneTreeNode* childNode = currNode->FindChild(*iter);
        if (!childNode)
            break;
        currNode = childNode;
    }
    return { currNode, tc::FStringUtils::Combine(iter, pathComps.end(), ".") };
}


void CScene::DFSTreeNodeUpdate(CSceneTreeNode* treeNode, bool markDirty)
{
    treeNode->L2WTransform.Update();

    const auto& childNodes = treeNode->GetChildren();
    for (CSceneTreeNode* child : childNodes)
        DFSTreeNodeUpdate(child, markDirty);

    if (auto* ent = treeNode->GetEntity())
    {
        // Update the instance entity
        if (ent->IsDirty() )
        {
            treeNode->SetEntityUpdated(true);
            markedDirty = markDirty;
            if (ent->IsMesh())
                brenderUpdate = false;
        }

        ent->UpdateEntity();
    }
}

void CScene::Update()
{
    // Called every frame to make sure everything is up to date
    brenderUpdate = true;
    DFSTreeNodeUpdate(GetRootTreeNode(), true);

    if (markedDirty && !Merges.empty() && !brenderUpdate)
    {
        for (auto & Merge : Merges)
        {
            auto* ent = dynamic_cast<Scene::CMeshMerger*>(Merge.second->GetEntity());
            if (ent != nullptr)
            {
                ent->MarkDirty();
                ent->MergeClear();
                for (auto &child : Merge.second->GetSceneNodeChildren())
                {
                    child->ForEachTreeNode([&](Scene::CSceneTreeNode* node) {
                        auto* entity = node->GetInstanceEntity(); // Else, get the instance
                        if (!entity) // Check to see if the an entity is instantiable (e.g., polyline, funnel, mesh, etc.), and not just an instance identifier.
                            entity = node->GetOwner()->GetEntity(); // If it's not instantiable, get entity instead of instance entity

                        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity)) { // set "auto * mesh" to this entity. Call MergeIn to set merger's vertices based on mesh's vertices. Reminder: an instance identifier is NOT a Mesh, so only real entities get merged.
                            ent->MergeIn(*mesh);
                            entity->isMerged = true;
                        }

                    });
                }
                ent->Catmull();
            }
        }

        DFSTreeNodeUpdate(GetRootTreeNode(), false);
    }
}

std::vector<CSceneTreeNode*> CScene::GetSelectedNodes() const
{
    // TODO: selection is not currently implemented, so returns the whole scene
    std::vector<CSceneTreeNode*> r;
    ForEachSceneTreeNode([&r](CSceneTreeNode* node) { r.push_back(node); });
    return r;
}

CPickingManager* CScene::GetPickingMgr() const { return PickingMgr; }


}

