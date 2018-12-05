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

    virtual void operator()(CEffiMesh& mesh) = 0;
};

} /* namespace Nome */
