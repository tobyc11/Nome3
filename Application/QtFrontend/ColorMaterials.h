#pragma once

#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QTechnique>

namespace Nome {

class CColorFlatMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT

public:
    explicit CColorFlatMaterial(Qt3DCore::QNode *parent = nullptr);
    ~CColorFlatMaterial() override = default;

    [[nodiscard]] QVariant BaseColor() const;

public Q_SLOTS:
    void SetBaseColor(const QVariant &baseColor);

Q_SIGNALS:
    void BaseColorChanged(const QVariant &baseColor);

private:
    Qt3DRender::QParameter* BaseColorParameter;
    Qt3DRender::QEffect* Effect;
    Qt3DRender::QTechnique* GL3Technique;
    Qt3DRender::QRenderPass* GL3RenderPass;
    Qt3DRender::QShaderProgram* GL3Shader;
    Qt3DRender::QFilterKey* FilterKey;
};

}
