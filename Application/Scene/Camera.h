#pragma once
#include "Entity.h"
#include <QQuaternion>


namespace Nome::Scene
{
    struct CCameraInfo
    {
        float para[6];
        std::string type;
        virtual ~CCameraInfo() = default;
    };

    class CCamera : public CEntity {
    DEFINE_INPUT(float, left) { MarkDirty(); }
    DEFINE_INPUT(float, right) { MarkDirty(); }
    DEFINE_INPUT(float, bottom) { MarkDirty(); }
    DEFINE_INPUT(float, top) { MarkDirty(); }
    DEFINE_INPUT(float, nearPlane) { MarkDirty(); }
    DEFINE_INPUT(float, farPlane) { MarkDirty(); }
        void MarkDirty() override;
        void UpdateEntity() override;

    public:
        DECLARE_META_CLASS(CCamera, CEntity);
        CCamera() = default;
        explicit CCamera(const std::string& name)
                : CEntity(std::move(name))
        {
        }

        bool IsMesh() override;
        bool IsInstantiable() override;
        CEntity* Instantiate(CSceneTreeNode* treeNode) override;
        CCameraInfo& GetCamera(){return CI;};



    private:
        CCameraInfo CI;
        CSceneTreeNode* SceneTreeNode;
    };

}
