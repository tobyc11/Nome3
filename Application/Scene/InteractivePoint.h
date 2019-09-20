#pragma once
#include "Scene.h"
#include <Color.h>
#include <Vector3.h>
#include <VectorPool.h>
#include <functional>
#include <unordered_map>
#include <string>

namespace Nome
{

//Forward Declaration
class CInteractivePoint;

class CPickingManager : public tc::FRefCounted
{
public:
    explicit CPickingManager(Scene::CScene* scene) : Scene(scene)
    {
    }

    uint32_t RegisterObj(CInteractivePoint* interactivePoint);
    void UnregisterObj(uint32_t id);

    //Synchronized picking
    CInteractivePoint* Pick(uint32_t x, uint32_t y);

private:
    Scene::CScene* Scene;

    std::unordered_map<uint32_t, CInteractivePoint*> PointMap;
    uint32_t NextObjId = 1;
};

//Implements a clickable point on the screen, can be subclassed
class CInteractivePoint
{
public:
    std::string GetName() const;
    void SetName(std::string value);

    tc::Color GetColor() const;
    void SetColor(tc::Color value);

    tc::Vector3 GetPosition() const;
    void SetPosition(tc::Vector3 value);

private:
    //Hopefully we can draw text at some point
    std::string Name;

    tc::Color Color;
    tc::Vector3 Position;

    friend class CPickingManager;
    uint32_t ObjId; //Uniquely identifies this guy in the scene, assigned by the manager
};

} /* namespace Nome */
