#pragma once
#include "GraphicsDevice.h"

namespace Nome
{

class CEffiContext
{
public:
    ///Create a context, auto select render device
    CEffiContext();

	CEffiContext(CGraphicsDevice* gd);
	
	~CEffiContext();

	CGraphicsDevice* GetGraphicsDevice() const
	{
		return GD;
	}

private:
	bool bDidCreateGD = false;
	CGraphicsDevice* GD;
};

} /* namespace Nome */
