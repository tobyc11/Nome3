#include "ASTContext.h"

namespace Nome::AST
{

CASTContext::CASTContext()
{
    Slabs.push_back(std::make_unique<char[]>(InitialSize));
    SlabLeft.push_back(InitialSize);
    SlabUsedCount.push_back(0);
    InitialSize *= 2;
}

CToken* CASTContext::MakeToken(std::string identifier)
{
    return Make<CToken>(std::move(identifier), -1, 0);
}

AIdent* CASTContext::MakeIdent(std::string identifier)
{
    return Make<AIdent>(MakeToken(std::move(identifier)));
}

}
