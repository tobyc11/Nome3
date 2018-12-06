#include "EffiCompiledPipeline.h"

namespace Nome
{

CEffiUnindexedMesh* CEffiCompiledPipeline::operator()(CEffiMesh& mesh)
{
	bool firstIteration = true;
	CEffiUnindexedMesh* next = nullptr;
	for (auto* a : MeshOperators)
	{
		if (firstIteration)
			next = a->operator()(mesh);
		else
			next = a->operator()(*next);
		firstIteration = false;
	}
	return next;
}

CEffiUnindexedMesh* CEffiCompiledPipeline::operator()(CEffiUnindexedMesh& mesh)
{
	bool firstIteration = true;
	CEffiUnindexedMesh* next = nullptr;
	for (auto* a : MeshOperators)
	{
		if (firstIteration)
			next = a->operator()(mesh);
		else
			next = a->operator()(*next);
		firstIteration = false;
	}
	return next;
}

} /* namespace Nome */
