#include "ExportSTL.h"
#include <cassert>
#include <cstdint>
#include <cstring>

using namespace Nome::Scene;

#pragma pack(push, 1)
struct STLHeader
{
    uint8_t Header[80];
    uint32_t TriangleCount;
};

struct STLTriangle
{
    Vector3 Normal;
    Vector3 V[3];
    uint16_t AttributeByteCount;
};
#pragma pack(pop)

static_assert(sizeof(STLHeader) == 84, "STLHeader wrong size");
static_assert(sizeof(STLTriangle) == 50, "STLTriangle wrong size");

CExportSTL::CExportSTL()
    : TriangleCount(0)
{
    Buffer.resize(sizeof(STLHeader));
    memset(Buffer.data(), 0, sizeof(STLHeader));
}

void CExportSTL::Process(const CMeshInstance& mesh)
{
    assert(!mesh.IsDirty());

    // Since MeshInstance::Mesh is in object local coordinates, we need the transformations from the scene
    const auto modelMat = mesh.GetSceneTreeNode()->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
    const auto normalMat = modelMat.ToMatrix3().Inverse().Transpose();

    const auto& m = mesh.Mesh;
    for (const auto& faceInfo : mesh.NameToFace)
    {
        const auto fHandle = faceInfo.second;
        STLTriangle triBuffer;
        triBuffer.AttributeByteCount = 0;
        int k = 0;

        const auto& normal = m.normal(fHandle);
        triBuffer.Normal = normalMat * Vector3(normal[0], normal[1], normal[2]);

        for (OpenMesh::PolyConnectivity::ConstFaceVertexIter fvIter = m.cfv_iter(fHandle); fvIter.is_valid(); ++fvIter)
        {
            const auto& point = m.point(*fvIter);
            triBuffer.V[k] = modelMat * Vector3(point[0], point[1], point[2]);
            k++;
            if (k == 3)
            {
                k = 2;

                // Append the triangle to Buffer
                size_t curOff = Buffer.size();
                Buffer.resize(curOff + sizeof(STLTriangle));
                memcpy(Buffer.data() + curOff, &triBuffer, sizeof(STLTriangle));

                // Update triangle count in STL header
                this->TriangleCount++;
                STLHeader* header = (STLHeader*)Buffer.data();
                header->TriangleCount = this->TriangleCount;

                // 012, 023, 034...
                triBuffer.V[1] = triBuffer.V[2];
            }
        }
    }
}
