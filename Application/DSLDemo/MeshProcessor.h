#pragma once

#include "Mesh.h"
#include "GraphicsDevice.h"

#include <Matrix4.h>

namespace Nome
{

using tc::Matrix4;

class CMeshProcessor
{
public:
	void ApplyMatrixToAttr(CEffiMesh& mesh, const std::string& attr, const Matrix4& matrix);

private:
	CGraphicsDevice* GD;
};

} /* namespace Nome */
