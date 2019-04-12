#pragma once
#include "AST.h"

namespace Nome
{

class CNomeDriver;

// A helper class that contains the arguments to a command, used by certain paresr rules
class ACommandExtHelper
{
public:
    std::map<AIdent*, AExpr*> NamedArgs;

    void MoveTo(CNomeDriver& driver, ACommand* dest);
};

class CNomeDriver
{
public:
    CNomeDriver(const std::string& sourceString);
    CNomeDriver(CASTContext* context, CSourceManager* sourceMgr, CSourceFile* sourceFile);

    void ParseToAST();

    CASTContext* GetASTContext() const { return Context; }
    CSourceLocation OffsetToLocation(int offset, int line = 0, int col = 0)
    {
        assert(line <= UINT16_MAX && line >= 0);
        assert(col <= UINT16_MAX && col >= 0);
        if (SourceMgr)
        {
            auto srcLoc = SourceMgr->GetLocation(SourceFile, offset);
            srcLoc.DebugLine = (uint16_t)line;
            srcLoc.DebugCol = (uint16_t)col;
            return srcLoc;
        }
        return CSourceLocation();
    }

    AIdent* MakeIdent(const std::string& identifier);
    void MoveIdentList(ACommand* dest, const std::string& paramName);
    void MoveFaceList(ACommand* dest);
    void MoveInstanceList(ACommand* dest);
    void MoveSetList(ACommand* dest);
    void MoveFaceForDeletionList(ACommand* dest);
    void MoveTransformList(ACommand* dest, const std::string& paramName);

    // Global vars used by the lexer
    int tokenBegin = 0;
    int tokenEnd = 0;
    int currOffset = 0;

    // Global vars used by the parser
    std::vector<Nome::AIdent*> IdentList;
    std::vector<Nome::ACommand*> FaceList;
    std::vector<Nome::ACommand*> InstanceList;
    std::vector<Nome::ACommand*> SetList;
    std::vector<Nome::ACommand*> FaceForDeletionList;
    std::vector<Nome::ATransform*> TransformList;
    ACommandExtHelper Ext;

private:
    void InvokeParser(void* scanner);

    CASTContext* Context;

    // Either the sourcemgr+file or the string is active
    CSourceManager* SourceMgr = nullptr;
    CSourceFile* SourceFile = nullptr;
    std::string SourceString;
};

}
