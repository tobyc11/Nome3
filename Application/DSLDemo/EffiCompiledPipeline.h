#pragma once
#include "MeshOperator.h"

#include <vector>

namespace Nome
{

class CEffiCompiledPipeline
{
public:
    void AddOperator(CMeshOperator* oper)
    {
        MeshOperators.push_back(oper);
    }

private:
    std::vector<CMeshOperator*> MeshOperators;
};

} /* namespace Nome */
