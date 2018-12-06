#include "EffiCompiledPipeline.h"

namespace Nome
{

void CEffiCompiledPipeline::operator()(CEffiMesh& mesh)
{
	for (auto* a : MeshOperators)
	{
		a->operator()(mesh);
	}
}

} /* namespace Nome */
