#pragma once
#include "AST.h"
#include <string>

namespace Nome
{

//One layer of indirection just in case when we modify the AST structure
typedef ACommand* CCommandHandle;

//TODO: implement this in terms of SourceManager and AST
class CCommandRewriter
{
public:
	CCommandRewriter(CCommandHandle handle);

	void Rename(const std::string& newName);

	void ClearArgs();
	void AppendArgs(const std::string& args);
};

}
