#pragma once
#include "MeshOperator.h"
#include "GraphicsDevice.h"

#include <string>
#include <map>

namespace Nome
{

class MOMaterializeAttr : public CMeshOperator
{
public:
	MOMaterializeAttr(CGraphicsDevice* gd, const std::string& mapFunc, const std::string& vsIn, const std::string& vsOut);

    void operator()(CEffiMesh& mesh) override;

private:
	//TODO: add constructor for these
	CGraphicsDevice* GD;
	std::string AttrName;
	std::map<std::string, EDataType> ReferredAttrs;
};

} /* namespace Nome */
