#include "SourceManager.h"

#include <string>
#include <fstream>
#include <streambuf>

namespace Nome
{

CSourceFile::CSourceFile()
{
    Content = new CRope();
}

CSourceFile::CSourceFile(const std::string& path, sp<CRope> content) : FilePath(path), Content(content)
{
}

CSourceManager::CSourceManager()
{
}

wp<CSourceFile> CSourceManager::OpenFile(const std::string& filePath)
{
    //Read the file into a string
    std::ifstream t(filePath);
    std::string str;

    t.seekg(0, std::ios::end);
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());

    sp<CRope> content = new CRope(str);
    sp<CSourceFile> file = new CSourceFile(filePath, content);
    if (!PrimaryFile.promote())
        PrimaryFile = file;
    Files.insert(file);
    return file;
}

wp<CSourceFile> CSourceManager::GetPrimaryFile() const
{
    return PrimaryFile;
}

}
