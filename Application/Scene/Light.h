#pragma once
#include "Entity.h"
#include <Flow/FlowNodeArray.h>

#include <map>
#include <set>
#include <utility>

namespace Nome::Scene
{

class CLightInstance;
struct CLightInfo
{
    Vector3 color;
    std::string type;
    std::string name;

    virtual ~CLightInfo() = default;
};


class CLight : public CEntity
{
DEFINE_INPUT(float, R) { MarkDirty(); }
DEFINE_INPUT(float, G) { MarkDirty(); }
DEFINE_INPUT(float, B) { MarkDirty(); }
DEFINE_INPUT(std::string, type) { MarkDirty(); }



public:
    DECLARE_META_CLASS(CLight, CEntity);
    CLight() = default;
    explicit CLight(std::string name)
        : CEntity(std::move(name))
    {
    }

    void MarkDirty() override;
    void UpdateEntity() override;
    bool IsInstantiable() override;
    bool IsMesh() override;
    CEntity* Instantiate(CSceneTreeNode* treeNode) override;
    AST::ACommand* SyncToAST(AST::CASTContext& ctx, bool createNewNode) override;


private:
    CLightInfo LI;

    friend class CLightInstance;
    std::set<CLightInstance*> InstanceSet;
};

class CLightInstance : public CEntity
{
public:
    using Super = CEntity;

    CLightInstance(CLight* generator, CSceneTreeNode* stn);
    ~CLightInstance() override;

    // Called when the Light entity is updated
    void MarkDirty() override;

    // Copy the actual Light from the Light entity and notify selectors
    void UpdateEntity() override;

    // Nothing changed about this mesh except its transformations and such
    void MarkOnlyDownstreamDirty();

    // Create a vertex selector with a vertex name, and a name for the resulting vertex

    // Get the scene tree node associated with this Light instance
    CSceneTreeNode* GetSceneTreeNode() const { return SceneTreeNode; }



private:
    // Instance specific data
    TAutoPtr<CLight> LightGenerator;
    /// A weak pointer to the owning scene tree node
    CSceneTreeNode* SceneTreeNode;

    unsigned int TransformChangeConnection;

    
};

}