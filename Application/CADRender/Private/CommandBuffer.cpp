#include "CommandBuffer.h"
#include "MaterialEnumsGL.h"
#include <glad/glad.h>

CCommandBuffer::CCommandBuffer()
{
    currentBuffers.resize(16);
    currentOffsets.assign(16, 0);
}

void CCommandBuffer::BindProgram(std::shared_ptr<CShaderProgram> program)
{
    lambdaQueue.emplace_back([program { std::move(program) }]() {
        GLuint id = program->GetProgramId();
        glUseProgram(id);
    });
}

void CCommandBuffer::BindMaterial(std::shared_ptr<CMaterial> material)
{
    currentMaterial = material;
    lambdaQueue.emplace_back([material { std::move(material) }]() { material->InternalBind(); });
}

void CCommandBuffer::SetParameterMap(std::unordered_map<int, CParameterValue> parameterMap)
{
    lambdaQueue.emplace_back([parameterMap { std::move(parameterMap) }]() {
        for (const auto& param : parameterMap)
        {
            param.second.BindFn(param.first);
        }
    });
}

void CCommandBuffer::SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height, float minDepth, float maxDepth)
{
    lambdaQueue.emplace_back([=]() { glViewport(x, y, width, height); });
}

void CCommandBuffer::BindVertexBuffers(uint32_t firstBinding, const std::vector<std::shared_ptr<CBuffer>>& buffers,
                                       const std::vector<uint32_t>& offsets)
{
    assert(buffers.size() == offsets.size());
    assert(firstBinding + buffers.size() <= currentBuffers.size());
    for (uint32_t i = firstBinding; i < firstBinding + buffers.size(); i++)
    {
        currentBuffers[i] = buffers[i];
        currentOffsets[i] = offsets[i];
    }
}

void CCommandBuffer::DrawPrimitives(EPrimitiveTopology topology, int vertexStart, uint32_t vertexCount)
{
    // Prepping the VAO now rather than later
    GLuint vaoId = 0;
    ParentCtx->GetDevice().QueueTask(
        [&vaoId, this]() {
            if (currentMaterial && !currentMaterial->attributeDescs.empty())
                vaoId = GenerateBindVAO(currentBuffers, currentOffsets, currentMaterial->bufferBindingDescs,
                                        currentMaterial->attributeDescs);
        },
        false, true);
    if (vaoId)
        vaosToDelete.push_back(vaoId);
    lambdaQueue.emplace_back([=]() {
        glBindVertexArray(vaoId);
        glDrawArrays(TrPrimitiveTopology(topology), vertexStart, vertexCount);
    });
}

void CCommandBuffer::DrawPrimitives(EPrimitiveTopology topology, int vertexStart, uint32_t vertexCount,
                                    uint32_t instanceCount)
{
    // Prepping the VAO now rather than later
    GLuint vaoId = 0;
    ParentCtx->GetDevice().QueueTask(
        [&vaoId, this]() {
            if (currentMaterial && !currentMaterial->attributeDescs.empty())
                vaoId = GenerateBindVAO(currentBuffers, currentOffsets, currentMaterial->bufferBindingDescs,
                                        currentMaterial->attributeDescs);
        },
        false, true);
    if (vaoId)
        vaosToDelete.push_back(vaoId);
    lambdaQueue.emplace_back([=]() {
        glBindVertexArray(vaoId);
        glDrawArraysInstanced(TrPrimitiveTopology(topology), vertexStart, vertexCount, instanceCount);
    });
}

void CCommandBuffer::DrawIndexedPrimitives(EPrimitiveTopology topology, uint32_t indexCount, EBaseType indexType,
                                           std::shared_ptr<CBuffer> indexBuffer, uint32_t indexBufferOffset)
{
    // Prepping the VAO now rather than later
    GLuint vaoId = 0;
    ParentCtx->GetDevice().QueueTask(
        [&vaoId, this]() {
            if (currentMaterial && !currentMaterial->attributeDescs.empty())
                vaoId = GenerateBindVAO(currentBuffers, currentOffsets, currentMaterial->bufferBindingDescs,
                                        currentMaterial->attributeDescs);
        },
        false, true);
    if (vaoId)
        vaosToDelete.push_back(vaoId);
    assert(indexType == EBaseType::UInt16 || indexType == EBaseType::UInt32);
    lambdaQueue.emplace_back([=, indexBuffer { std::move(indexBuffer) }]() {
        glBindVertexArray(vaoId);
        GLenum type = indexType == EBaseType::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
        glDrawElements(TrPrimitiveTopology(topology), indexCount, type, (const void*)(uintptr_t)indexBufferOffset);
    });
}

void CCommandBuffer::Submit()
{
    currentMaterial.reset();
    currentBuffers.resize(16);
    currentOffsets.assign(16, 0);
    ParentCtx->GetDevice().QueueTask(
        [lambdaQueue { std::move(lambdaQueue) }, vaosToDelete { std::move(vaosToDelete) }]() {
            for (const auto& lambda : lambdaQueue)
            {
                lambda();
            }
            CMaterial::RestoreGLStatesInternal();
            glBindVertexArray(0);
            glDeleteVertexArrays(vaosToDelete.size(), vaosToDelete.data());
        });
}

unsigned int CCommandBuffer::GenerateBindVAO(const std::vector<std::shared_ptr<CBuffer>>& boundBuffers,
                                             const std::vector<uint32_t>& boundOffsets,
                                             const std::vector<CBufferBindingDesc>& boundBufferBindings,
                                             const std::vector<CAttributeDesc>& boundAttribs)
{
    // Generate VAO based on the bound buffers and the VertexInputState in the material
    GLuint vaoId;
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);
    for (const auto& vb : boundBufferBindings)
    {
        assert(boundBuffers[vb.Binding] && "No buffer bound at this slot");
        auto bufId = boundBuffers[vb.Binding]->InternalGetBufferHandle().BufferId;
        auto bufOffset = boundOffsets[vb.Binding];
        glBindBuffer(GL_ARRAY_BUFFER, bufId);
        for (const auto& attrDesc : boundAttribs)
        {
            if (attrDesc.Binding != vb.Binding)
                continue;

            GLuint location = static_cast<uint32_t>(attrDesc.Attribute);
            if (location >= static_cast<uint32_t>(EVertexAttribute::Index0))
                location -= static_cast<uint32_t>(EVertexAttribute::Index0);
            glEnableVertexAttribArray(location);
            switch (attrDesc.BaseType)
            {
            case EBaseType::Float:
                glVertexAttribPointer(location, attrDesc.Components, GL_FLOAT, GL_FALSE, vb.Stride,
                                      reinterpret_cast<const void*>(attrDesc.Offset + bufOffset));
                break;
            case EBaseType::Int8:
            case EBaseType::UInt8:
            case EBaseType::Int16:
            case EBaseType::UInt16:
            case EBaseType::Int32:
            case EBaseType::UInt32:
                /*
                #define GL_BYTE 0x1400
                #define GL_UNSIGNED_BYTE 0x1401
                #define GL_SHORT 0x1402
                #define GL_UNSIGNED_SHORT 0x1403
                #define GL_INT 0x1404
                #define GL_UNSIGNED_INT 0x1405
                 We can do the conversion to GL by adding 0x1400
                 */
                static_assert(static_cast<int>(EBaseType::Int8) == 0, "EBaseType must agree with OpenGL");
                static_assert(static_cast<int>(EBaseType::UInt32) == 5, "EBaseType must agree with OpenGL");
                glVertexAttribIPointer(location, attrDesc.Components, static_cast<int>(attrDesc.BaseType) + 0x1400,
                                       vb.Stride, reinterpret_cast<const void*>(attrDesc.Offset + bufOffset));
                break;
            case EBaseType::Double:
                glVertexAttribLPointer(location, attrDesc.Components, GL_DOUBLE, vb.Stride,
                                       reinterpret_cast<const void*>(attrDesc.Offset + bufOffset));
                break;
            default:
                goto error;
            }
            if (vb.InputRate != 0)
                glVertexAttribDivisor(location, vb.InputRate);
        }
    }
    return vaoId;

error:
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vaoId);
    return 0;
}
