#pragma once
#include "Material.h"
#include "Shaders.h"
#include <functional>
#include <vector>

// Record commands for submission to the GPU
class CCommandBuffer : public CRenderContextChild
{
public:
    CCommandBuffer();

    void BindProgram(std::shared_ptr<CShaderProgram> program);
    void BindMaterial(std::shared_ptr<CMaterial> material);
    void SetParameterMap(std::unordered_map<int, CParameterValue> parameterMap);
    void SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height, float minDepth, float maxDepth);
    void BindVertexBuffers(uint32_t firstBinding, const std::vector<std::shared_ptr<CBuffer>>& buffers,
                           const std::vector<uint32_t>& offsets);

    // Draw
    void DrawPrimitives(EPrimitiveTopology topology, int vertexStart, uint32_t vertexCount);
    void DrawPrimitives(EPrimitiveTopology topology, int vertexStart, uint32_t vertexCount, uint32_t instanceCount);
    void DrawIndexedPrimitives(EPrimitiveTopology topology, uint32_t indexCount, EBaseType indexType,
                               std::shared_ptr<CBuffer> indexBuffer, uint32_t indexBufferOffset);

    void Submit();

protected:
    // Utilities to be invoked during GL processing time
    static unsigned int GenerateBindVAO(const std::vector<std::shared_ptr<CBuffer>>& boundBuffers,
                                        const std::vector<uint32_t>& boundOffsets,
                                        const std::vector<CBufferBindingDesc>& boundBufferBindings,
                                        const std::vector<CAttributeDesc>& boundAttribs);

private:
    std::vector<std::function<void()>> lambdaQueue;
    std::shared_ptr<CMaterial> currentMaterial;
    std::vector<std::shared_ptr<CBuffer>> currentBuffers;
    std::vector<uint32_t> currentOffsets;
    std::vector<unsigned int> vaosToDelete;
};
