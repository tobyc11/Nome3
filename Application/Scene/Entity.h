#pragma once
#include "Flow/FlowNode.h"
#include "RendererInterface.h"
#include <Matrix3x4.h> //For convenience
#include <string>

namespace Nome::Scene
{

//For convenience, so that all sub-classes can use unqualified names
using tc::Vector3;
using tc::Quaternion;
using tc::Matrix3x4;
using tc::TAutoPtr;

class CSceneTreeNode;

//Represents an object in the scene, can be attached to a SceneNode
class CEntity : public Flow::CFlowNode
{
public:
    CEntity();

    CEntity(std::string name) : Name(std::move(name)) {}

    ~CEntity() override
    {
    }

    const std::string& GetName() const { return Name; }
    //Don't use this, otherwise the Scene's EntityLibrary won't be updated with the new name
    void SetName(const std::string& value) { Name = value; }

    //The entity itself can also be considered an output, the following 2 functions handle the update thereof
    virtual void MarkDirty() { bEntityDirty = true; }
    //Update the entity, doesn't do anything if not dirty
    virtual void UpdateEntity() { if (!IsDirty()) return; bEntityDirty = false; UpdateCount++; }
    bool IsDirty() const { return bEntityDirty; }

    uint32_t GetUpdateCount() const { return UpdateCount; }

    bool IsEntityValid() const { return bIsValid; }
    void SetValid(bool value) { bIsValid = value; }

    virtual void Draw(IDebugDraw* draw) {};

    //Some entities(generators) allow actual instance objects for each scene tree node
    //  so that each instance can be customized, like delete face
    virtual bool IsInstantiable() { return false; }
    virtual CEntity* Instantiate(CSceneTreeNode* treeNode) { return nullptr; }

private:
    std::string Name;
    bool bIsValid = false;
    bool bEntityDirty = true;

    //For profiling
    uint32_t UpdateCount = 0;
};

} /* namespace Nome::Scene */
