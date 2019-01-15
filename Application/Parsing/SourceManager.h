#pragma once
#include "Rope.h"
#include <RefBase.h>
#include <map>
#include <set>
#include <string>

namespace Nome
{

using tc::sp;
using tc::wp;

class CSourceFile : public tc::RefBase
{
public:
    CSourceFile();
    CSourceFile(const std::string& path, sp<CRope> content);

	sp<CRope> GetAsRope() const { return Content; }

private:
    std::string FilePath;
    sp<CRope> Content;
};

class CSourceManager : public tc::TLightRefBase<CSourceManager>
{
public:
    CSourceManager();

    wp<CSourceFile> OpenFile(const std::string& filePath);
    wp<CSourceFile> GetPrimaryFile() const;

private:
    wp<CSourceFile> PrimaryFile;
    std::set<sp<CSourceFile>> Files;
};

class CSourceLocation
{
public:
private:
    CRope::CLocation Loc;
};

class CSourceRange
{
public:
private:
    CSourceLocation Begin;
    CSourceLocation End;
};

}
