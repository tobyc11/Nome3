#pragma once
#include "Buffer.h"
#include "BufferGeometry.h"
#include "CommandBuffer.h"
#include "DrawList.h"
#include "Material.h"

class CGridObject : public CRenderContextChild
{
public:
    explicit CGridObject();

    [[nodiscard]] int GetLineSegmentCount() const { return LineSegmentCount; }
    [[nodiscard]] float GetSpacing() const { return LineSpacing; }

    void RegenerateGeometry(int count, float spacing);
    void Draw(CCommandBuffer& cmdBuffer, const CCameraParams& cameraParams) const;

private:
    void MakeGeometry();
    void MakeMaterial();

    std::shared_ptr<CBufferGeometry> Geometry;
    std::shared_ptr<CMaterial> Material;
    int LineSegmentCount = 6;
    float LineSpacing = 1.0f;
};
