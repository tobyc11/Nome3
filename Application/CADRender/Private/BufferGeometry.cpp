#include "BufferGeometry.h"

std::vector<CBufferBindingDesc> CBufferGeometry::GetBufferBindings() const
{
    std::vector<CBufferBindingDesc> result;
    uint32_t bindingIndex = 0;
    for (const auto& bufferDesc : Buffers)
    {
        result.push_back({ bindingIndex, bufferDesc.Stride, bufferDesc.InputRate });
        bindingIndex += 1;
    }
    return result;
}

void CBufferGeometry::BindVertexBuffers(CCommandBuffer& cmdBuffer) const
{
    std::vector<std::shared_ptr<CBuffer>> buffersToBind;
    std::vector<uint32_t> offsetsToBind;
    for (const auto& bufferDesc : this->GetBuffers())
    {
        buffersToBind.push_back(bufferDesc.Buffer);
        offsetsToBind.push_back(bufferDesc.Offset);
    }
    cmdBuffer.BindVertexBuffers(0, buffersToBind, offsetsToBind);
}
