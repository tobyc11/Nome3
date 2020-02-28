#pragma once
#include "ASTBinding.h"
#include "Flow/FlowNode.h"
#include "RendererInterface.h"
#include <Matrix3x4.h> //For convenience
#include <string>

/*
 * The meta class provides basic reflection support for entities
 * Things we need:
 *  - Command name
 *  - Factory
 *  - AST to property bindings
 */
#define DECLARE_META_CLASS(TClass, TSuper)                                                         \
    using Super = TSuper;                                                                          \
    class CMetaClass : public IMetaClass, public CASTBinding<TClass>                               \
    {                                                                                              \
    public:                                                                                        \
        CMetaClass();                                                                              \
        std::string ClassName() override { return #TClass; }                                       \
        CEntity* MakeEntity() override { return new TClass; }                                      \
        bool DeserializeFromAST(AST::ACommand& cmd, CEntity& entity) override                      \
        {                                                                                          \
            return FromASTToObject(cmd, dynamic_cast<TClass&>(entity));                            \
        }                                                                                          \
    };                                                                                             \
    friend class CMetaClass;                                                                       \
    static CMetaClass MetaObject;                                                                  \
    IMetaClass& GetMetaObject() const override { return MetaObject; }

#define DEFINE_META_OBJECT(TClass)                                                                 \
    TClass::CMetaClass TClass::MetaObject;                                                         \
    TClass::CMetaClass::CMetaClass()

namespace Nome::Scene
{

// For convenience, so that all sub-classes can use unqualified names
using tc::Matrix3x4;
using tc::Quaternion;
using tc::TAutoPtr;
using tc::Vector3;

class CEntity;
class CSceneTreeNode;

// An abstract interface for the meta-class system for entity reflection
class IMetaClass
{
public:
    virtual std::string ClassName() = 0;
    virtual CEntity* MakeEntity() = 0;
    virtual bool DeserializeFromAST(AST::ACommand& cmd, CEntity& entity) = 0;
};

// Represents an object in the scene, can be attached to a SceneNode
class CEntity : public Flow::CFlowNode
{
public:
    // This chunk should be kept in sync with DECLARE_META_CLASS above
    // except some changes for CEntity being the root of all entities
    class CMetaClass : public IMetaClass, public CASTBinding<CEntity, false>
    {
    public:
        CMetaClass();
        std::string ClassName() override { return "CEntity"; }
        CEntity* MakeEntity() override { return new CEntity; }
        bool DeserializeFromAST(AST::ACommand& cmd, CEntity& entity) override
        {
            return FromASTToObject(cmd, entity);
        }
    };
    friend class CMetaClass;
    static CMetaClass MetaObject;
    virtual IMetaClass& GetMetaObject() const { return MetaObject; }

    CEntity();

    explicit CEntity(std::string name)
        : Name(std::move(name))
    {
    }

    ~CEntity() override = default;

    const std::string& GetName() const { return Name; }
    std::string GetNameWithoutPrefix() const;
    // Don't use this, otherwise the Scene's EntityLibrary won't be updated with the new name
    void SetName(const std::string& value) { Name = value; }

    // The entity itself can also be considered an output, the following 2 functions handle the
    // update thereof
    virtual void MarkDirty() { bEntityDirty = true; }
    // Update the entity, doesn't do anything if not dirty
    virtual void UpdateEntity()
    {
        if (!IsDirty())
            return;
        bEntityDirty = false;
        UpdateCount++;
    }
    bool IsDirty() const { return bEntityDirty; }

    uint32_t GetUpdateCount() const { return UpdateCount; }

    bool IsEntityValid() const { return bIsValid; }
    void SetValid(bool value) { bIsValid = value; }

    virtual void Draw(IDebugDraw* draw) {};

    // Some entities(generators) allow actual instance objects for each scene tree node
    //  so that each instance can be customized, like delete face
    virtual bool IsInstantiable() { return false; }
    virtual CEntity* Instantiate(CSceneTreeNode* treeNode) { return nullptr; }

private:
    std::string Name;
    bool bIsValid = false;
    bool bEntityDirty = true;

    // For profiling
    uint32_t UpdateCount = 0;
};

} /* namespace Nome::Scene */
