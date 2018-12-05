#include "Effi.h"

namespace Nome
{

CEffiContext::CEffiContext()
{
	GD = new CGraphicsDevice();
	bDidCreateGD = true;
}

CEffiContext::CEffiContext(CGraphicsDevice* gd) : GD(gd)
{
}

CEffiContext::~CEffiContext()
{
	if (bDidCreateGD)
		delete GD;
	GD = nullptr;
}

} /* namespace Nome */
