#pragma once
#include "MeshOperator.h"

#include <vector>

namespace Nome
{

class CEffiMesh;

class CEffiCompiledPipeline
{
public:
    void AddOperator(CMeshOperator* oper)
    {
        MeshOperators.push_back(oper);
    }

	CEffiUnindexedMesh* operator()(CEffiMesh& mesh);
	CEffiUnindexedMesh* operator()(CEffiUnindexedMesh& mesh);

private:
    std::vector<CMeshOperator*> MeshOperators;
};

} /* namespace Nome */
