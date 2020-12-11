#include "Material.h"
#include "MaterialEnumsGL.h"
#include <glad/glad.h>

void CParameterValue::BindFn(int index) const
{
    switch (ParamType)
    {
    case EParameterType::Vec3:
        glUniform3f(index, DataArr[0], DataArr[1], DataArr[2]);
        break;
    case EParameterType::Vec4:
        glUniform4f(index, DataArr[0], DataArr[1], DataArr[2], DataArr[3]);
        break;
    case EParameterType::Mat3:
        glUniformMatrix3fv(index, 1, GL_TRUE, (GLfloat*)&DataArr);
        break;
    case EParameterType::Mat4:
        glUniformMatrix4fv(index, 1, GL_TRUE, (GLfloat*)&DataArr);
        break;
    case EParameterType::UniformBuffer:
        index = index & (~0xff010000);
        glBindBufferBase(GL_UNIFORM_BUFFER, index + 1, Buffer->InternalGetBufferHandle().BufferId);
    }
}

std::unordered_map<int, CParameterValue> ParameterMapNew() { return {}; }

void ParameterMapAppend3f(std::unordered_map<int, CParameterValue>& map, int index, const float* data)
{
    if (index == -1)
        return;
    auto& value = map[index];
    value.ParamType = EParameterType::Vec3;
    value.DataArr[0] = data[0];
    value.DataArr[1] = data[1];
    value.DataArr[2] = data[2];
}

void ParameterMapAppend4f(std::unordered_map<int, CParameterValue>& map, int index, const float* data)
{
    if (index == -1)
        return;
    auto& value = map[index];
    value.ParamType = EParameterType::Vec4;
    value.DataArr[0] = data[0];
    value.DataArr[1] = data[1];
    value.DataArr[2] = data[2];
    value.DataArr[3] = data[3];
}

void ParameterMapAppendMatrix3fv(std::unordered_map<int, CParameterValue>& map, int index, uint32_t count,
                                 const float* data)
{
    assert(count == 1); // More than one Matrix is not yet supported
    if (index == -1)
        return;
    auto& value = map[index];
    value.ParamType = EParameterType::Mat3;
    for (int i = 0; i < 3 * 3; i++)
        value.DataArr[i] = data[i];
}

void ParameterMapAppendMatrix4fv(std::unordered_map<int, CParameterValue>& map, int index, uint32_t count,
                                 const float* data)
{
    assert(count == 1); // More than one Matrix is not yet supported
    if (index == -1)
        return;
    auto& value = map[index];
    value.ParamType = EParameterType::Mat4;
    for (int i = 0; i < 4 * 4; i++)
        value.DataArr[i] = data[i];
}

void ParameterMapAppendUniformBuffer(std::unordered_map<int, CParameterValue>& map, int index,
                                     std::shared_ptr<CBuffer> buffer)
{
    assert(index >= 0);
    auto& value = map[0xff010000 | index];
    value.ParamType = EParameterType::UniformBuffer;
    value.Buffer = std::move(buffer);
}

void CMaterial::InternalBind() const
{
    glUseProgram(ShaderProgram->GetProgramId());
    if (ShaderProgram->GetProgramId() == 0)
    {
        printf("Material: ProgramId == 0, something is off\n");
    }

    for (const auto& param : ParameterMap)
    {
        const auto& name = param.first;
        GLint location = ShaderProgram->GetUniformLocation(name);
        param.second.BindFn(location);
    }

    if (!Viewports.empty())
    {
        const auto& vp = *Viewports.begin();
        glViewport(vp.X, vp.Y, vp.Width, vp.Height);
        // TODO: glDepthRangef(vp.MinDepth, vp.MaxDepth);
        if (Viewports.size() > 1)
        {
            printf("We dont support more than 1 viewport yet\n");
        }
    }

    if (!Scissors.empty())
        printf("Scissors are not supported");

    if (cullMode == 0)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);
    if (frontFaceCW)
        glFrontFace(GL_CW);
    else
        glFrontFace(GL_CCW);
    if (polygonMode == EPolygonMode::Fill)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else if (polygonMode == EPolygonMode::Line)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if (polygonMode == EPolygonMode::Point)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    glLineWidth(lineWidth);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    if (depthBiasEnable)
    {
        glPolygonOffset(depthBiasSlopeFactor, depthBiasConstantFactor);
        if (GL_EXT_polygon_offset_clamp)
            glPolygonOffsetClamp(depthBiasSlopeFactor, depthBiasConstantFactor, depthBiasClamp);
        glEnable(GL_POLYGON_OFFSET_POINT);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glEnable(GL_POLYGON_OFFSET_FILL);
    }
    else
    {
        glDisable(GL_POLYGON_OFFSET_POINT);
        glDisable(GL_POLYGON_OFFSET_LINE);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    if (depthTestEnable)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(TrCompareOp(depthCompareOp));
    }
    else
        glDisable(GL_DEPTH_TEST);
}

void CMaterial::RestoreGLStatesInternal()
{
    glUseProgram(0);
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_POLYGON_OFFSET_POINT);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_DEPTH_TEST);
}
