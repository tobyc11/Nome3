#include "DynamicScene.h"
#include <Device.h>
#include <RHIInstance.h>
#include <UniformBuffer.h>

namespace Nome::Render
{

struct CSceneObjectParams
{
    alignas(RHI::TUniformBufferMemberTraits<tc::Matrix4>::Align)
        tc::Matrix4 ModelMat;
    alignas(RHI::TUniformBufferMemberTraits<tc::Matrix4>::Align)
        tc::Matrix4 NormalMat;

    static constexpr size_t RoundSize()
    {
        return (sizeof(CSceneObjectParams) + 15) / 16 * 16;
    }
};

tc::sp<RHI::CBuffer> CSceneObject::SceneObjectParamsBuffer;

CSceneObject::CSceneObject(const std::shared_ptr<CShape>& shape,
    const std::shared_ptr<CMaterial>& material,
    const tc::Matrix3x4& objToWorld)
    : Shape(shape)
    , Material(material)
    , ObjToWorld(objToWorld)
    , SceneObjectParams(std::make_unique<CSceneObjectParams>())
{
    NormalToWorld = objToWorld.ToMatrix3().Inverse().Transpose();
}

CSceneObject::~CSceneObject()
{
}

tc::BoundingBox CSceneObject::WorldBound() const
{
    return Shape->ObjectBound().Transformed(ObjToWorld);
}

void CSceneObject::BindSceneObjectParamsBuffer(RHI::CDrawTemplate& draw)
{
    if (!SceneObjectParamsBuffer)
    {
        SceneObjectParams->ModelMat = ObjToWorld.ToMatrix4();
        SceneObjectParams->NormalMat = NormalToWorld.ToMatrix4();
        SceneObjectParamsBuffer = RHI::CInstance::Get().GetCurrDevice()->CreateBuffer(
            (uint32_t)SceneObjectParams->RoundSize(), RHI::EBufferUsageFlags::ConstantBuffer, &SceneObjectParams);
    }
    else
    {
        SceneObjectParams->ModelMat = ObjToWorld.ToMatrix4();
        SceneObjectParams->NormalMat = NormalToWorld.ToMatrix4();
        RHI::CBufferUpdateRequest req;
        req.Buffer = SceneObjectParamsBuffer;
        req.Data = SceneObjectParams.get();
        req.Size = (uint32_t)SceneObjectParams->RoundSize();
        draw.GetBufferUpdateReqs().push_back(req);
    }
    draw.GetPipelineArguments().Add("SceneObjectParams"_hash, SceneObjectParamsBuffer);
}

CDynamicScene::CDynamicScene()
{
    PipelineCache = RHI::CInstance::Get().GetCurrDevice()->CreatePipelineCache();
}

CDynamicScene::~CDynamicScene()
{
    ClearObjectDrawCaches();
}

void CDynamicScene::AddObject(const std::string& name,
    const std::shared_ptr<CSceneObject>& obj)
{
    Objects[name] = obj;
}

void CDynamicScene::RemoveObject(const std::string& name)
{
    Objects.erase(name);
}

void CDynamicScene::UpdateObjectToWorld(const std::string& name,
    const tc::Matrix3x4& objToWorld)
{
    Objects[name]->ObjToWorld = objToWorld;
    Objects[name]->NormalToWorld = objToWorld.ToMatrix3().Inverse().Transpose();
}

std::vector<CSceneObject*> CDynamicScene::Cull(const tc::Frustum& frustum) const
{
    //TODO: actually cull objects
    std::vector<CSceneObject*> result;
    for (const auto& pair : Objects)
    {
        result.push_back(&*pair.second);
    }
    return result;
}

void CDynamicScene::ClearObjectDrawCaches()
{
    ObjectDrawDescs.clear();
    for (auto pair : ObjectPipelines)
        PipelineCache->DestroyPipelineStates(pair.second);
    ObjectPipelines.clear();
}

} // namespace Nome::Render
