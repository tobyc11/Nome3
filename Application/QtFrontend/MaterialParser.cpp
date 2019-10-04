#include "MaterialParser.h"
#include <QFile>
#include <QUrl>
#include <QVector3D>
#include <QVector4D>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureData>
#include <pugixml.hpp>
#include <fstream>
#include <sstream>
#include <utility>

namespace Nome
{

CMaterialParser::CMaterialParser(QString path)
    : Path(std::move(path))
{
}

CMaterialParser::~CMaterialParser()
{
    if (ResultMaterial)
    {
        if (!ResultMaterial->parent())
            delete ResultMaterial;
    }
}

bool CMaterialParser::Parse()
{
    using namespace Qt3DRender;

    QFile inputFile{Path};
    if (!inputFile.open(QIODevice::ReadOnly))
        return false;
    auto fileBytes = inputFile.readAll();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(fileBytes.data(), fileBytes.size());
    if (!result)
    {
        return false;
    }

    //There should only be one material node per document
    pugi::xml_node materialNode = doc.child("Material");
    auto* material = new QMaterial();
    for (auto paramNode : materialNode.children("Parameter"))
    {
        auto* parameter = new QParameter();
        material->addParameter(parameter);
        parameter->setName(paramNode.attribute("name").value());
        parameter->setValue(ParseVariant(paramNode.attribute("value").value()));
    }
    for (auto effectNode : materialNode.children("Effect"))
    {
        auto* effect = new QEffect();
        material->setEffect(effect);
        for (auto paramNode : effectNode.children("Parameter"))
        {
            auto* parameter = new QParameter();
            effect->addParameter(parameter);
            parameter->setName(paramNode.attribute("name").value());
            parameter->setValue(ParseVariant(paramNode.attribute("value").value()));
        }
        for (auto techNode : effectNode.children("Technique"))
        {
            auto* technique = new QTechnique();
            effect->addTechnique(technique);

            for (auto filterKeyNode : techNode.children("FilterKey"))
            {
                auto* filterKey = new QFilterKey();
                technique->addFilterKey(filterKey);
                filterKey->setName(filterKeyNode.attribute("name").value());
                filterKey->setValue(filterKeyNode.attribute("value").value());
            }

            auto graphicsApiFilterNode = techNode.child("GraphicsApiFilter");
            if (graphicsApiFilterNode)
            {
                //TODO:
                technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGL);
                technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::CoreProfile);
                technique->graphicsApiFilter()->setMajorVersion(3);
                technique->graphicsApiFilter()->setMinorVersion(1);
            }
            else
                goto bail;

            for (auto renderPassNode : techNode.children("RenderPass"))
            {
                auto* renderPass = new QRenderPass();
                technique->addRenderPass(renderPass);

                auto shaderProgramNode = renderPassNode.child("ShaderProgram");
                if (!shaderProgramNode)
                    goto bail;
                auto* shaderProgram = new QShaderProgram();
                renderPass->setShaderProgram(shaderProgram);
                auto vertexShaderCode = shaderProgramNode.attribute("vertexShaderCode");
                if (vertexShaderCode)
                {
                    shaderProgram->setVertexShaderCode(QShaderProgram::loadSource(QUrl(vertexShaderCode.value())));
                }
                auto geometryShaderCode = shaderProgramNode.attribute("geometryShaderCode");
                if (geometryShaderCode)
                {
                    shaderProgram->setGeometryShaderCode(QShaderProgram::loadSource(QUrl(geometryShaderCode.value())));
                }
                auto fragmentShaderCode = shaderProgramNode.attribute("fragmentShaderCode");
                if (fragmentShaderCode)
                {
                    shaderProgram->setFragmentShaderCode(QShaderProgram::loadSource(QUrl(fragmentShaderCode.value())));
                }
            }
        }
    }

    ResultMaterial = material;
    return true;

    bail:
    delete material;
    return false;
}

Qt3DRender::QMaterial* CMaterialParser::GetResultMaterial() const
{
    return ResultMaterial;
}

QVariant CMaterialParser::ParseVariant(const char* str)
{
    std::string source(str);
    size_t cutoff = source.find_first_of(':');
    std::string type = source.substr(0, cutoff);
    std::string value = source.substr(cutoff + 1);
    std::stringstream ss{value};
    float f0, f1, f2, f3;
    std::string fileName;
    if (type == "float")
    {
        ss >> f0;
        return f0;
    }
    else if (type == "vec3")
    {
        ss >> f0 >> f1 >> f2;
        return QVector3D{f0, f1, f2};
    }
    else if (type == "vec4")
    {
        ss >> f0 >> f1 >> f2 >> f3;
        return QVector4D{f0, f1, f2, f3};
    }
    else if (type == "texture2d")
    {
        auto* tex = new Qt3DRender::QTexture2D();
        tex->setMinificationFilter(Qt3DRender::QAbstractTexture::LinearMipMapLinear);
        tex->setMagnificationFilter(Qt3DRender::QAbstractTexture::Linear);
        tex->wrapMode()->setX(Qt3DRender::QTextureWrapMode::Repeat);
        tex->wrapMode()->setY(Qt3DRender::QTextureWrapMode::Repeat);
        tex->setGenerateMipMaps(true);
        tex->setMaximumAnisotropy(16.0f);

        ss >> fileName;
        fileName = "file:" + fileName;
        auto* image = new Qt3DRender::QTextureImage();
        tex->addTextureImage(image);
        image->setSource(QUrl(fileName.c_str()));
        return QVariant::fromValue(tex);
    }
    return QVariant();
}

}
