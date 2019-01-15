#pragma once
#include "Flow/FlowNode.h"
#include <Matrix3x4.h> //For convenience
#include <string>

namespace Nome::Scene
{

//For convenience, so that all sub-classes can use unqualified names
using tc::Vector3;
using tc::Quaternion;
using tc::Matrix3x4;
using tc::TAutoPtr;

//Represents an object in the scene, can be attached to a SceneNode
class CEntity : public Flow::CFlowNode
{
	//This output represents the entity
    DEFINE_OUTPUT_WITH_UPDATE(CEntity*, This)
    {
        UpdateEntity();
    }

public:
    CEntity();

    CEntity(std::string name) : Name(std::move(name)) {}

    ~CEntity() override
    {
    }

    const std::string& GetName() const { return Name; }
    //Don't use this, otherwise the Scene's EntityLibrary won't be updated with the new name
    void SetName(const std::string& value) { Name = value; }

	//Handle update of the entire entity, intended to be overridden by subclasses
    virtual void MarkDirty() { This.MarkDirty(); }
	virtual void UpdateEntity() { This.UnmarkDirty(); }

    bool IsEntityValid() const { return bIsValid; }
    void SetValid(bool value) { bIsValid = value; }

	//Some entities(generators) allow actual instance objects for each scene tree node
	//  so that each instance can be customized, like delete face
	virtual bool IsInstantiable() { return false; }
	virtual CEntity* Instantiate() { return nullptr; }

private:
    std::string Name;
    bool bIsValid = false;
};

} /* namespace Nome::Scene */
