#pragma once
#include "SyntaxTree.h"
#include <vector>

namespace Nome
{

// Keeps track of a sequence of tokens together to ease handling
class CTokenList
{
public:

private:
    std::vector<AST::CToken*> Tokens;
};

}
