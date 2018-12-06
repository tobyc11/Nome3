#pragma once
#include "MeshOperator.h"
#include "GraphicsDevice.h"
#include "IR.h"

#include <string>
#include <map>

namespace Nome
{

class MOMaterializeAttr : public CMeshOperator
{
public:
	MOMaterializeAttr(CGraphicsDevice* gd, const std::string& attrName, IRExpr* targetExpr);

    void operator()(CEffiMesh& mesh) override;

private:
	CGraphicsDevice* GD;
	std::string AttrName;
	EDataType OutputType;
	std::map<unsigned, std::pair<std::string, EDataType>> ReferredAttrs; //Ordered by attribute index

	ID3D11InputLayout* InputLayout;
	ID3D11VertexShader* VertexShader;
	ID3D11GeometryShader* GeometryShader;
};

} /* namespace Nome */
