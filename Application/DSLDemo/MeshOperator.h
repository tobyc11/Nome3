#pragma once
#include "Mesh.h"

#include <stdexcept>

namespace Nome
{

class CMeshOpException : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

class CMeshOperator
{
public:
    virtual ~CMeshOperator() = default;

    virtual CEffiUnindexedMesh* operator()(CEffiMesh& mesh) = 0;
	virtual CEffiUnindexedMesh* operator()(CEffiUnindexedMesh& mesh) = 0;
};

} /* namespace Nome */
