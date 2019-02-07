#include "ASTUtils.h"

namespace Nome
{

CCommandHandle::CCommandHandle(ACommand* cmd, CSourceManager* sm, CSourceFile* sf)
    : Cmd(cmd), SourceManager(sm), SourceFile(sf)
{
}

CCommandRewriter::CCommandRewriter(CCommandHandle& handle) : CmdHandle(handle)
{
}

void CCommandRewriter::Rename(const std::string& newName)
{
    CSourceRange range{ CmdHandle.Cmd->Name->BeginLoc, CmdHandle.Cmd->Name->EndLoc };
    CmdHandle.SourceManager->ReplaceRange(CmdHandle.SourceFile, range, newName);
}

void CCommandRewriter::ReplaceArg(int index, const std::string & content)
{
    CSourceRange range{ CmdHandle.Cmd->Args[index]->BeginLoc, CmdHandle.Cmd->Args[index]->EndLoc };
    CmdHandle.SourceManager->ReplaceRange(CmdHandle.SourceFile, range, content);
}

}
