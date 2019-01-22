#pragma once
#include "Point.h"
#include <Flow/FlowNodeArray.h>

namespace Nome::Scene
{

class CMesh;

class CFace : public CEntity
{
    //Face points, in order
	DEFINE_INPUT_ARRAY(CVertexInfo*, Points) { MarkDirty(); }

	DEFINE_OUTPUT_WITH_UPDATE(CFace*, Face) { UpdateEntity(); }

public:
	using Super = CEntity;
    CFace() {}
    CFace(std::string name) : CEntity(std::move(name)) {}

	void MarkDirty() override;
	void UpdateEntity() override;

    size_t CountVertices() const;

	void AddFaceIntoMesh(CMesh* mesh) const;
};

}
