#pragma once
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>

#include <string>

namespace Nome
{

class CMaterialParser
{
public:
    explicit CMaterialParser(QString path, Qt3DRender::QMaterial* parseInto = nullptr);
    ~CMaterialParser();

    bool Parse();
    [[nodiscard]] Qt3DRender::QMaterial* GetResultMaterial() const;

    static QVariant ParseVariant(const char* str);

private:
    QString Path;
    bool bDoNotAutoDelete = false;
    Qt3DRender::QMaterial* ResultMaterial = nullptr;
};

class CXMLMaterial : public Qt3DRender::QMaterial
{
public:
    explicit CXMLMaterial(QString path);

    [[nodiscard]] Qt3DRender::QParameter* FindParameterByName(const QString& name) const;
};

}
