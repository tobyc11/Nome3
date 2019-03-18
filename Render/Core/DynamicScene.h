#pragma once
#include "Materials/Material.h"
#include "Shapes/Shape.h"

#include <Buffer.h>
#include <Device.h>
#include <PipelineCache.h>
#include <Frustum.h>
#include <Matrix3x4.h>

#include <string>
#include <unordered_map>

namespace Nome::Render
{

struct CSceneObjectParams;

class CSceneObject
{
public:
    CSceneObject(const std::shared_ptr<CShape>& shape,
        const std::shared_ptr<CMaterial>& material,
        const tc::Matrix3x4& objToWorld);
    ~CSceneObject();

    tc::BoundingBox WorldBound() const;
    const std::shared_ptr<CShape> GetShape() const { return Shape; }
    const std::shared_ptr<CMaterial> GetMaterial() const { return Material; }

    void BindSceneObjectParamsBuffer(RHI::CDrawTemplate& draw);

private:
    friend class CDynamicScene;

    tc::Matrix3x4 ObjToWorld;
    tc::Matrix3x4 NormalToWorld;

    static tc::sp<RHI::CBuffer> SceneObjectParamsBuffer;
    std::unique_ptr<CSceneObjectParams> SceneObjectParams;

    std::shared_ptr<CShape> Shape;
    std::shared_ptr<CMaterial> Material;
};

class CDynamicScene
{
    friend class CRenderer;

public:
    CDynamicScene();
    ~CDynamicScene();

    CDynamicScene(const CDynamicScene&) = delete;
    CDynamicScene(CDynamicScene&&) = delete;
    CDynamicScene& operator=(const CDynamicScene&) = delete;
    CDynamicScene& operator=(CDynamicScene&&) = delete;

    void AddObject(const std::string& name,
        const std::shared_ptr<CSceneObject>& obj);
    void RemoveObject(const std::string& name);
    void UpdateObjectToWorld(const std::string& name,
        const tc::Matrix3x4& objToWorld);

    std::vector<CSceneObject*> Cull(const tc::Frustum& frustum) const;

    void ClearObjectDrawCaches();

private:
    std::unordered_map<std::string, std::shared_ptr<CSceneObject>> Objects;
    tc::sp<RHI::CPipelineCache> PipelineCache;

    std::unordered_map<CSceneObject*, RHI::CDrawTemplate> ObjectDrawDescs;
    std::unordered_map<CSceneObject*, RHI::CPipelineStates> ObjectPipelines;
};

} // namespace Nome::Render
