#pragma once

#include "Flow/FlowNode.h"

//For convenience
#include <Matrix3x4.h>

#include <string>
#include <cassert>

namespace Nome
{

class CDocument;

namespace Scene
{

//For convenience, so that all sub-classes can use unqualified names
using tc::Vector3;
using tc::Quaternion;
using tc::Matrix3x4;

class CSceneNode;

class CEntity : public Flow::CFlowNode
{
public:
    CEntity(CDocument* doc, std::string name)
        : Document(doc), Name(std::move(name))
    {
        static unsigned int nameCounter = 1;
        if (Name.empty())
        {
            Name = "entity" + std::to_string(nameCounter++);
        }
    }

    ~CEntity()
    {
        assert(Document == nullptr);
        assert(ParentSceneNode == nullptr);
    }

    const std::string& GetName() const
    {
        return Name;
    }

    void SetName(const std::string& value)
    {
        Name = value;
    }

protected:
    ///Marks the entity dirty, triggers a scene update (redraw)
    void MarkDirty();

private:
    std::string Name;

    ///Pointer to the document this entity belongs to
    CDocument* Document;

    ///Indicates whether this entity is part of the scene
    CSceneNode* ParentSceneNode = nullptr;

    bool bIsDirty = true;
};

} /* namespace Scene */
} /* namespace Nome */
