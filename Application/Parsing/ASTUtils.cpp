#include "ASTUtils.h"

namespace Nome
{

CCommandHandle::CCommandHandle(ACommand* cmd, CSourceManager* sm, CSourceFile* sf)
    : Cmd(cmd)
    , SourceManager(sm)
    , SourceFile(sf)
{
}

CCommandRewriter::CCommandRewriter(CCommandHandle& handle)
    : CmdHandle(handle)
{
}

void CCommandRewriter::Rename(const std::string& newName)
{
    CSourceRange range { CmdHandle.Cmd->GetName()->BeginLoc, CmdHandle.Cmd->GetName()->EndLoc };
    CmdHandle.SourceManager->ReplaceRange(CmdHandle.SourceFile, range, newName);
}

void CCommandRewriter::ReplaceArg(const std::string& name, const std::string& content)
{
    CSourceRange range { CmdHandle.Cmd->FindNamedArg(name)->BeginLoc,
                         CmdHandle.Cmd->FindNamedArg(name)->EndLoc };
    CmdHandle.SourceManager->ReplaceRange(CmdHandle.SourceFile, range, content);
}

}
