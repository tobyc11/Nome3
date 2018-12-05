#include "MeshProcessor.h"
#include "ShaderManager.h"

namespace Nome
{

void CMeshProcessor::ApplyMatrixToAttr(CEffiMesh& mesh, const std::string& attr, const Matrix4& matrix)
{
	mesh.SyncAttrToGPU(attr);
	auto& vertAttr = mesh.VertexAttrs[attr];

	D3D11_INPUT_ELEMENT_DESC vsInputs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	GD->GetDevice()->CreateInputLayout(vsInputs, ARRAYSIZE(vsInputs), nullptr, 0, nullptr);
}

} /* namespace Nome */
