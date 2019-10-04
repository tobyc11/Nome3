#pragma once
#include <Qt3DRender/QMaterial>

#include <string>

namespace Nome
{

class CMaterialParser
{
public:
    explicit CMaterialParser(QString path);
    ~CMaterialParser();

    bool Parse();
    [[nodiscard]] Qt3DRender::QMaterial* GetResultMaterial() const;

    static QVariant ParseVariant(const char* str);

private:
    QString Path;
    Qt3DRender::QMaterial* ResultMaterial = nullptr;
};

}
