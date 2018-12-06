#pragma once
#include "MeshOperator.h"
#include "GraphicsDevice.h"
#include "IR.h"

#include <string>
#include <map>

namespace Nome
{

class MOSubdivideAdHoc : public CMeshOperator
{
public:
	MOSubdivideAdHoc(CGraphicsDevice* gd, IRExpr* inputExpr);

	CEffiUnindexedMesh* operator()(CEffiMesh& mesh) override;
	CEffiUnindexedMesh* operator()(CEffiUnindexedMesh& mesh) override;

private:
	CGraphicsDevice* GD;
	std::map<unsigned, std::pair<std::string, EDataType>> ReferredAttrs; //Ordered by attribute index

	ID3D11InputLayout* InputLayout;
	ID3D11VertexShader* VertexShader;
	ID3D11GeometryShader* GeometryShader;
};

} /* namespace Nome */
