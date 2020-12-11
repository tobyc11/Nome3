#include "RenderComponent.h"

namespace Nome::Scene
{

bool CRenderComponent::OnAttach()
{
    // Check the entity attached to is an instance in the scene tree
    auto* meshInst = dynamic_cast<CMeshInstance*>(GetEntity());
    if (!meshInst)
    {
        printf("Cannot attach render component to anything other than CMeshInstance\n");
        return false;
    }
    MeshInst = meshInst;
    SceneTreeNode = meshInst->GetSceneTreeNode();
    // Add this into the scene-wide render component list
    CScene* scene = SceneTreeNode->GetOwner()->GetScene();
    scene->GetRenderCompList().insert(this);
    return true;
}

void CRenderComponent::OnBeforeDetach()
{
    // Remove this from scene-wide render component list
    CScene* scene = SceneTreeNode->GetOwner()->GetScene();
    scene->GetRenderCompList().erase(this);
}

void CMeshRenderComponent::Update()
{
    MeshInst->UpdateEntity();
    CMeshImpl& mesh = MeshInst->Mesh;

    struct CVertexData
    {
        Vector3 Pos;
        Vector3 Normal;
    };
    static_assert(sizeof(CVertexData) == sizeof(float) * 6, "Vector3 size should be 6x float");
    std::vector<CVertexData> vertexDataArr;
    VertexCount = 0;

    CMeshImpl::FaceIter fIter, fEnd = mesh.faces_end();
    for (fIter = mesh.faces_sbegin(); fIter != fEnd; ++fIter)
    {
        CVertexData v0, vPrev, vCurr;
        int fvIndex = 0;
        CMeshImpl::FaceVertexIter fvIter = CMeshImpl::FaceVertexIter(mesh, *fIter);
        for (; fvIter.is_valid(); ++fvIter)
        {
            CMeshImpl::VertexHandle faceVert = *fvIter;
            if (fvIndex == 0)
            {
                const auto& posVec = mesh.point(faceVert);
                v0.Pos = { posVec[0], posVec[1], posVec[2] };
                const auto& fnVec = bSmoothShading ? mesh.normal(faceVert) : mesh.normal(*fIter);
                v0.Normal = { fnVec[0], fnVec[1], fnVec[2] };
            }
            else if (fvIndex == 1)
            {
                const auto& posVec = mesh.point(faceVert);
                vPrev.Pos = { posVec[0], posVec[1], posVec[2] };
                const auto& fnVec = bSmoothShading ? mesh.normal(faceVert) : mesh.normal(*fIter);
                vPrev.Normal = { fnVec[0], fnVec[1], fnVec[2] };
            }
            else
            {
                const auto& posVec = mesh.point(faceVert);
                vCurr.Pos = { posVec[0], posVec[1], posVec[2] };
                const auto& fnVec = bSmoothShading ? mesh.normal(faceVert) : mesh.normal(*fIter);
                vCurr.Normal = { fnVec[0], fnVec[1], fnVec[2] };
                vertexDataArr.push_back(v0);
                vertexDataArr.push_back(vPrev);
                vertexDataArr.push_back(vCurr);
                VertexCount += 3;
                vPrev = vCurr;
            }
            fvIndex++;
        }
    }
    if (VertexCount != 0)
    {
        auto buffer = std::make_shared<CBuffer>(EBufferType::Vertex, vertexDataArr.size() * sizeof(CVertexData),
                                                vertexDataArr.data());
        CBufferDesc bufferDescs[] = { { buffer, 0, sizeof(CVertexData), 0 } };
        CAttributeDesc attribDescs[] = { { EVertexAttribute::Position, 0, EBaseType::Float, 3, 0 },
                                         { EVertexAttribute::Normal, 0, EBaseType::Float, 3, 12 } };
        Geometry = std::make_shared<CBufferGeometry>(bufferDescs, attribDescs, EPrimitiveTopology::TriangleList);
    }
    else
    {
        Geometry.reset();
    }

    // Generate another geometry for all the line segments in polyline (if any)
    const std::vector<CMeshImpl::VertexHandle>& lineStrip = MeshInst->MeshGenerator->LineStrip;
    std::vector<CVertexData> lineBuffer;
    if (!lineStrip.empty())
    {
        for (size_t i = 1; i < lineStrip.size(); i++)
        {
            const auto& vPos0 = MeshInst->MeshGenerator->Mesh.point(lineStrip[i - 1]);
            const auto& vPos1 = MeshInst->MeshGenerator->Mesh.point(lineStrip[i]);
            lineBuffer.push_back({ Vector3(vPos0[0], vPos0[1], vPos0[2]), Vector3() });
            lineBuffer.push_back({ Vector3(vPos1[0], vPos1[1], vPos1[2]), Vector3() });
            LinesVertexCount += 2;
        }
    }

    if (LinesVertexCount != 0)
    {
        auto buffer =
            std::make_shared<CBuffer>(EBufferType::Vertex, lineBuffer.size() * sizeof(CVertexData), lineBuffer.data());
        CBufferDesc bufferDescs[] = { { buffer, 0, sizeof(CVertexData), 0 } };
        CAttributeDesc attribDescs[] = { { EVertexAttribute::Position, 0, EBaseType::Float, 3, 0 },
                                         { EVertexAttribute::Normal, 0, EBaseType::Float, 3, 12 } };
        LinesGeometry = std::make_shared<CBufferGeometry>(bufferDescs, attribDescs, EPrimitiveTopology::LineList);
    }
    else
    {
        LinesGeometry.reset();
    }
}

void CMeshRenderComponent::Draw(CDrawList& drawList)
{
    drawList.SetModelMat(SceneTreeNode->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY).ToMatrix4());
    if (Geometry)
    {
        drawList.DrawGeometry(*Geometry, VertexCount);
        drawList.DrawWireframe(*Geometry, VertexCount);
    }
    if (LinesGeometry)
    {
        drawList.DrawWireframe(*LinesGeometry, LinesVertexCount);
    }
}

}
