#pragma once
#include "AST.h"
#include <string>

namespace Nome
{

// One layer of indirection just in case when we modify the AST structure
struct CCommandHandle
{
    CCommandHandle(ACommand* cmd, CSourceManager* sm, CSourceFile* sf);

    ACommand* Cmd;
    sp<CSourceManager> SourceManager;
    CSourceFile* SourceFile;
};

class CCommandRewriter
{
public:
    CCommandRewriter(CCommandHandle& handle);

    void Rename(const std::string& newName);

    void ReplaceArg(const std::string& name, const std::string& content);

private:
    CCommandHandle& CmdHandle;
};

}
