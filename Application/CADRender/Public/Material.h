#pragma once
#include "Buffer.h"
#include "MaterialEnums.h"
#include "Shaders.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

struct CViewport
{
    float X;
    float Y;
    float Width;
    float Height;
    float MinDepth;
    float MaxDepth;
};

struct COffset2D
{
    int32_t X;
    int32_t Y;
};

struct CExtent2D
{
    uint32_t Width;
    uint32_t Height;
};

struct CRect2D
{
    COffset2D Offset;
    CExtent2D Extent;
};

struct CStencilOpState
{
    EStencilOp FailOp;
    EStencilOp PassOp;
    EStencilOp DepthFailOp;
    ECompareOp CompareOp;
    uint32_t CompareMask;
    uint32_t WriteMask;
    uint32_t Reference;
};

struct VkPipelineColorBlendAttachmentState
{
    bool blendEnable;
    VkBlendFactor srcColorBlendFactor;
    VkBlendFactor dstColorBlendFactor;
    VkBlendOp colorBlendOp;
    VkBlendFactor srcAlphaBlendFactor;
    VkBlendFactor dstAlphaBlendFactor;
    VkBlendOp alphaBlendOp;
    VkColorComponentFlags colorWriteMask;
};

enum EParameterType
{
    Vec3,
    Vec4,
    Mat3,
    Mat4,
    UniformBuffer
};

struct CParameterValue
{
    void BindFn(int index) const;

    EParameterType ParamType;
    std::shared_ptr<CBuffer> Buffer;
    union
    {
        void* DataPtr;
        float DataArr[16];
    };
};

std::unordered_map<int, CParameterValue> ParameterMapNew();

void ParameterMapAppend3f(std::unordered_map<int, CParameterValue>& map, int index, const float* data);
void ParameterMapAppend4f(std::unordered_map<int, CParameterValue>& map, int index, const float* data);
// The matrices are column major
void ParameterMapAppendMatrix3fv(std::unordered_map<int, CParameterValue>& map, int index, uint32_t count,
                                 const float* data);
void ParameterMapAppendMatrix4fv(std::unordered_map<int, CParameterValue>& map, int index, uint32_t count,
                                 const float* data);
void ParameterMapAppendUniformBuffer(std::unordered_map<int, CParameterValue>& map, int index,
                                     std::shared_ptr<CBuffer> buffer);

class CMaterial
{
public:
    std::shared_ptr<CShaderProgram> ShaderProgram;
    std::unordered_map<std::string, CParameterValue> ParameterMap;

    // VertexInputState (buffer bindings and attributes)
    std::vector<CBufferBindingDesc> bufferBindingDescs;
    std::vector<CAttributeDesc> attributeDescs;
    unsigned int cachedVAO = 0;

    // InputAssemblyState (topology is included with draw calls)
    // Omitting PrimitiveRestartEnable

    // TessellationState
    uint32_t PatchControlPoints;

    // ViewportState
    std::vector<CViewport> Viewports;
    std::vector<CRect2D> Scissors;

    // RasterizationState
    bool depthClampEnable;
    bool rasterizerDiscardEnable;
    EPolygonMode polygonMode;
    uint32_t cullMode;
    bool frontFaceCW;
    bool depthBiasEnable;
    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;

    void SetDepthBias(float slopeFactor, float constantFactor, float clamp)
    {
        depthBiasEnable = true;
        depthBiasSlopeFactor = slopeFactor;
        depthBiasConstantFactor = constantFactor;
        depthBiasClamp = clamp;
    }

    void DisableDepthBias() { depthBiasEnable = false; }

    // DepthStencilState
    bool depthTestEnable = false;
    bool depthWriteEnable = true;
    ECompareOp depthCompareOp = ECompareOp::Less;
    bool depthBoundsTestEnable;
    bool stencilTestEnable;
    CStencilOpState front;
    CStencilOpState back;
    float minDepthBounds;
    float maxDepthBounds;

    // ColorBlendState
    bool logicOpEnable;
    VkLogicOp logicOp;
    uint32_t attachmentCount;
    const VkPipelineColorBlendAttachmentState* pAttachments;
    float blendConstants[4];

    void InternalBind() const;
    static void RestoreGLStatesInternal();
};
