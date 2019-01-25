#include "NomeDriver.h"

namespace Nome
{

void ACommandExtHelper::MoveTo(ACommand* dest)
{
    dest->Args.insert(dest->Args.end(), Args.begin(), Args.end());
    dest->NamedArgs.insert(NamedArgs.begin(), NamedArgs.end());
    *this = ACommandExtHelper();
}

CNomeDriver::CNomeDriver(const std::string& sourceString) : SourceString(sourceString)
{
    Context = new CASTContext();
}

CNomeDriver::CNomeDriver(CASTContext* context, CSourceManager* sourceMgr, CSourceFile* sourceFile)
	: Context(context), SourceMgr(sourceMgr), SourceFile(sourceFile)
{
}

}
