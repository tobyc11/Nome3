#pragma once
#include "AST.h"
#include "Rope.h"
#include "SourceManager.h"

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
    CNomeDriver(CASTContext* context, CRope* source);

    void ParseToAST();

    CASTContext* GetASTContext() const { return Context; }
    CRope::CLocation OffsetToLocation(int offset)
    {
        return CRope::CLocation();
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

    //Only one of the following is active
	CRope* Source = nullptr;
    std::string SourceString;
};

}
