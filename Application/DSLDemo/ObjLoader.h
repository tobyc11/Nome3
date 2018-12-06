#pragma once
#include "Mesh.h"

#include <string>

namespace Nome
{

class CObjLoader
{
public:
	CObjLoader(const std::string& fileName);

	CEffiMesh* LoadEffiMesh(CGraphicsDevice* gd);

private:
	std::string FileName;
};

} /* namespace Nome */
