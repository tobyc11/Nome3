#include "MaterialParser.h"
#include <QFile>
#include <QUrl>
#include <QVector3D>
#include <QVector4D>
#include <Qt3DRender/Qt3DRender>
#include <fstream>
#include <pugixml.hpp>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace Nome
{

CMaterialParser::CMaterialParser(QString path, Qt3DRender::QMaterial* parseInto)
    : Path(std::move(path))
    , ResultMaterial(parseInto)
{
    if (parseInto)
        bDoNotAutoDelete = true;
}

CMaterialParser::~CMaterialParser()
{
    if (!bDoNotAutoDelete && ResultMaterial)
    {
        if (!ResultMaterial->parent())
            delete ResultMaterial;
    }
}

bool CMaterialParser::Parse()
{
    using namespace Qt3DRender;

    QFile inputFile { Path };
    if (!inputFile.open(QIODevice::ReadOnly))
        return false;
    auto fileBytes = inputFile.readAll();
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(fileBytes.data(), fileBytes.size());
    if (!result)
    {
        return false;
    }

    // There should only be one material node per document
    pugi::xml_node materialNode = doc.child("Material");
    auto* material = ResultMaterial ? ResultMaterial : new QMaterial();
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
                // TODO:
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
                    shaderProgram->setVertexShaderCode(
                        QShaderProgram::loadSource(QUrl(vertexShaderCode.value())));
                }
                auto geometryShaderCode = shaderProgramNode.attribute("geometryShaderCode");
                if (geometryShaderCode)
                {
                    shaderProgram->setGeometryShaderCode(
                        QShaderProgram::loadSource(QUrl(geometryShaderCode.value())));
                }
                auto fragmentShaderCode = shaderProgramNode.attribute("fragmentShaderCode");
                if (fragmentShaderCode)
                {
                    shaderProgram->setFragmentShaderCode(
                        QShaderProgram::loadSource(QUrl(fragmentShaderCode.value())));
                }

                auto cullFaceNode = renderPassNode.child("CullFace");
                if (cullFaceNode)
                {
                    const std::unordered_map<std::string, QCullFace::CullingMode> modeMap = {
                        { "NoCulling", QCullFace::NoCulling },
                        { "Front", QCullFace::Front },
                        { "Back", QCullFace::Back },
                        { "FrontAndBack", QCullFace::FrontAndBack }
                    };

                    auto* cullState = new QCullFace();
                    std::string mode = cullFaceNode.attribute("mode").value();
                    cullState->setMode(modeMap.at(mode));
                    renderPass->addRenderState(cullState);
                }

                auto blendNode = renderPassNode.child("Blend");
                if (blendNode)
                {
                    const std::unordered_map<std::string, QBlendEquation::BlendFunction> opMap = {
                        { "Add", QBlendEquation::Add },
                        { "Subtract", QBlendEquation::Subtract },
                        { "ReverseSubtract", QBlendEquation::ReverseSubtract },
                        { "Min", QBlendEquation::Min },
                        { "Max", QBlendEquation::Max }
                    };

                    const std::unordered_map<std::string, QBlendEquationArguments::Blending>
                        factorMap = {
                            { "Add", QBlendEquationArguments::Zero },
                            { "One", QBlendEquationArguments::One },
                            { "SourceColor", QBlendEquationArguments::SourceColor },
                            { "SourceAlpha", QBlendEquationArguments::SourceAlpha },
                            { "DestinationColor", QBlendEquationArguments::DestinationColor },
                            { "DestinationAlpha", QBlendEquationArguments::DestinationAlpha },
                            { "SourceAlphaSaturate", QBlendEquationArguments::SourceAlphaSaturate },
                            { "ConstantColor", QBlendEquationArguments::ConstantColor },
                            { "ConstantAlpha", QBlendEquationArguments::ConstantAlpha },
                            { "OneMinusSourceColor", QBlendEquationArguments::OneMinusSourceColor },
                            { "OneMinusSourceAlpha", QBlendEquationArguments::OneMinusSourceAlpha },
                            { "OneMinusDestinationAlpha",
                              QBlendEquationArguments::OneMinusDestinationAlpha },
                            { "OneMinusDestinationColor",
                              QBlendEquationArguments::OneMinusDestinationColor },
                            { "OneMinusConstantColor",
                              QBlendEquationArguments::OneMinusConstantColor },
                            { "OneMinusConstantAlpha",
                              QBlendEquationArguments::OneMinusConstantAlpha }
                        };

                    auto* blendFunc = new QBlendEquation();
                    std::string op = blendNode.attribute("op").value();
                    blendFunc->setBlendFunction(opMap.at(op));
                    renderPass->addRenderState(blendFunc);

                    auto* blendWeights = new QBlendEquationArguments();
                    std::string srcFactor = blendNode.attribute("srcFactor").value();
                    std::string dstFactor = blendNode.attribute("dstFactor").value();
                    blendWeights->setSourceRgba(factorMap.at(srcFactor));
                    blendWeights->setDestinationRgba(factorMap.at(dstFactor));
                    renderPass->addRenderState(blendWeights);
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

Qt3DRender::QMaterial* CMaterialParser::GetResultMaterial() const { return ResultMaterial; }

QVariant CMaterialParser::ParseVariant(const char* str)
{
    std::string source(str);
    size_t cutoff = source.find_first_of(':');
    std::string type = source.substr(0, cutoff);
    std::string value = source.substr(cutoff + 1);
    std::stringstream ss { value };
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
        return QVector3D { f0, f1, f2 };
    }
    else if (type == "vec4")
    {
        ss >> f0 >> f1 >> f2 >> f3;
        return QVector4D { f0, f1, f2, f3 };
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
        fileName = fileName;
        auto* image = new Qt3DRender::QTextureImage();
        tex->addTextureImage(image);
        image->setSource(QUrl(fileName.c_str()));
        return QVariant::fromValue(tex);
    }
    return QVariant();
}

CXMLMaterial::CXMLMaterial(QString path)
{
    CMaterialParser parser { path, this };
    parser.Parse();
}

Qt3DRender::QParameter* CXMLMaterial::FindParameterByName(const QString& name) const
{
    for (Qt3DRender::QParameter* param : this->parameters())
    {
        if (param->name() == name)
            return param;
    }
    return nullptr;
}

}
