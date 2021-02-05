#include "ColorMaterials.h"

#include <QColor>
#include <QUrl>
#include <QVector3D>
#include <QVector4D>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>

namespace Nome
{

CColorFlatMaterial::CColorFlatMaterial(Qt3DCore::QNode* parent)
    : QMaterial(parent)
{
    using namespace Qt3DRender;

    BaseColorParameter = new QParameter(QStringLiteral("baseColor"), QColor("white"));
    Effect = new QEffect();
    GL3Technique = new QTechnique();
    GL3RenderPass = new QRenderPass();
    GL3Shader = new QShaderProgram();
    FilterKey = new QFilterKey();

    QObject::connect(BaseColorParameter, &QParameter::valueChanged, this,
                     &CColorFlatMaterial::BaseColorChanged);
    GL3Shader->setVertexShaderCode(
        QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/Shaders/default.vert"))));
    GL3Shader->setFragmentShaderCode(
        QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/Shaders/ColorFlat.frag"))));

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

QVariant CColorFlatMaterial::BaseColor() const { return BaseColorParameter->value(); }

void CColorFlatMaterial::SetBaseColor(const QVariant& baseColor)
{
    BaseColorParameter->setValue(baseColor);
}

CWireframeEffect::CWireframeEffect(Qt3DCore::QNode* parent)
    : QEffect(parent)
{
    using namespace Qt3DRender;
    /* Randy commented out on 10/11. This code doesn't seem to do anything?? I think logic is stored
    in MaterialParser.cpp auto* GL3Technique = new QTechnique();
    GL3Technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGL);
    GL3Technique->graphicsApiFilter()->setMajorVersion(3);
    GL3Technique->graphicsApiFilter()->setMinorVersion(1);
    GL3Technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::CoreProfile);
    auto* FilterKey = new QFilterKey();
    FilterKey->setParent(GL3Technique);
    FilterKey->setName(QStringLiteral("renderingStyle"));
    FilterKey->setValue(QStringLiteral("forward"));
    GL3Technique->addFilterKey(FilterKey);
    auto* GL3RenderPass = new QRenderPass();
    auto* GL3Shader = new QShaderProgram();

    GL3Shader->setVertexShaderCode(
        QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/Shaders/Wireframe.vert"))));
    GL3Shader->setGeometryShaderCode(
        QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/Shaders/Wireframe.geom"))));
    GL3Shader->setFragmentShaderCode(
        QShaderProgram::loadSource(QUrl(QStringLiteral("qrc:/Shaders/Wireframe.frag"))));
    GL3RenderPass->setShaderProgram(GL3Shader);
    GL3Technique->addRenderPass(GL3RenderPass);
    this->addTechnique(GL3Technique);*/
}

CWireframeMaterial::CWireframeMaterial(Qt3DCore::QNode* parent)
    : QMaterial(parent)
{
    using namespace Qt3DRender;
    /*  Randy commented out on 10/11. This code doesn't seem to do anything?? I think logic is
    stored in MaterialParser.cpp this->addParameter(new QParameter(QStringLiteral("ka"),
    QVector3D(0.1f, 0.1f, 0.1f))); this->addParameter(new QParameter(QStringLiteral("kd"),
    QVector3D(0.7f, 0.7f, 0.7f))); this->addParameter(new QParameter(QStringLiteral("ks"),
    QVector3D(0.95f, 0.95f, 0.95f))); this->addParameter(new QParameter(QStringLiteral("shininess"),
    150.0f)); this->addParameter( new QParameter(QStringLiteral("light.position"), QVector4D(0.0f,
    0.0f, 0.0f, 1.0f))); this->addParameter( new QParameter(QStringLiteral("light.intensity"),
    QVector3D(1.0f, 1.0f, 1.0f))); this->addParameter(new
    QParameter(QStringLiteral("line.width"), 1.0f)); this->addParameter( new
    QParameter(QStringLiteral("line.color"), QVector4D(1.0f, 1.0f, 1.0f, 1.0f)));

    this->setEffect(new CWireframeEffect(this));
       */
}

}