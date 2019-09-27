#include "ColorMaterials.h"

#include <QColor>
#include <QUrl>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QGraphicsApiFilter>

namespace Nome {

CColorFlatMaterial::CColorFlatMaterial(Qt3DCore::QNode* parent)
    : QMaterial(parent)
{
    using namespace Qt3DRender;

    BaseColorParameter = new QParameter(QStringLiteral("baseColor"), QColor("white"));
    Effect=new QEffect();
    GL3Technique = new QTechnique();
    GL3RenderPass = new QRenderPass();
    GL3Shader = new QShaderProgram();
    FilterKey = new QFilterKey();

    QObject::connect(BaseColorParameter, &QParameter::valueChanged, this, &CColorFlatMaterial::BaseColorChanged);
    GL3Shader->setVertexShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/Shaders/default.vert"))));
    GL3Shader->setFragmentShaderCode(QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/Shaders/ColorFlat.frag"))));

    GL3Technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGL);
    GL3Technique->graphicsApiFilter()->setMajorVersion(3);
    GL3Technique->graphicsApiFilter()->setMinorVersion(1);
    GL3Technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::CoreProfile);

    FilterKey->setParent(this);
    FilterKey->setName(QStringLiteral("renderingStyle"));
    FilterKey->setValue(QStringLiteral("forward"));

    GL3Technique->addFilterKey(FilterKey);
    GL3RenderPass->setShaderProgram(GL3Shader);
    GL3Technique->addRenderPass(GL3RenderPass);
    Effect->addTechnique(GL3Technique);

    Effect->addParameter(BaseColorParameter);

    this->setEffect(Effect);
}

QVariant CColorFlatMaterial::BaseColor() const
{
    return BaseColorParameter->value();
}

void CColorFlatMaterial::SetBaseColor(const QVariant& baseColor)
{
    BaseColorParameter->setValue(baseColor);
}

}
