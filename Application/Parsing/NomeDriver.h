#pragma once
#include "AST.h"

namespace Nome
{

//A helper class that contains the arguments to a command, used by certain paresr rules
class ACommandExtHelper
{
public:
    std::vector<AExpr*> Args;
    std::map<AKeyword*, AExpr*> NamedArgs;

    void MoveTo(ACommand* dest);
};

class CNomeDriver
{
public:
    CNomeDriver(const std::string& sourceString);
    CNomeDriver(CASTContext* context, CSourceManager* sourceMgr, CSourceFile* sourceFile);

    void ParseToAST();

    CASTContext* GetASTContext() const { return Context; }
    CSourceLocation OffsetToLocation(int offset)
    {
		if (SourceMgr)
			return SourceMgr->GetLocation(SourceFile, offset);
		return CSourceLocation();
    }

    //Global vars used by the lexer
    int tokenBegin = 0;
    int tokenEnd = 0;
    int currOffset = 0;

	//Global vars used by the parser
	std::vector<Nome::AIdent*> IdentList;
	std::vector<Nome::ACommand*> FaceList;
    std::vector<Nome::ACommand*> InstanceList;
    std::vector<Nome::ACommand*> SetList;
    ACommandExtHelper Ext;

private:
	void InvokeParser(void* scanner);

    CASTContext* Context;

    //Either the sourcemgr+file or the string is active
	CSourceManager* SourceMgr = nullptr;
	CSourceFile* SourceFile = nullptr;
    std::string SourceString;
};

}
