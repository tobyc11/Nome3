#include "Shaders.h"
#include <glad/glad.h>
#include <utility>

int EShaderStageToGL(EShaderStage input)
{
    switch (input)
    {
    case EShaderStage::Vertex:
        return GL_VERTEX_SHADER;
    case EShaderStage::Fragment:
        return GL_FRAGMENT_SHADER;
    default:
        return -1;
    }
}

CShaderObject::CShaderObject(CGLThread* device, EShaderStage type, std::string source)
    : Device(device)
    , Type(type)
    , Source(std::move(source))
{
    if (Compile())
    {
        Cleanup();
    }
}

CShaderObject::~CShaderObject()
{
    Device->QueueTask(
        [this]() {
            if (ShaderId)
                glDeleteShader(ShaderId);
        },
        false, true);
}

bool CShaderObject::IsValid() const { return ShaderId != 0; }

bool CShaderObject::Compile()
{
    ShaderIdReady = Device->QueueTask([this]() {
        auto shaderId = glCreateShader(EShaderStageToGL(this->Type));
        const GLchar* srcArray[] = { this->Source.c_str() };
        const int srcLengths[] = { (int)this->Source.length() };
        GLint status;
        glShaderSource(shaderId, 1, srcArray, srcLengths);
        glCompileShader(shaderId);
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE)
        {
            char logBuffer[1024];
            glGetShaderInfoLog(shaderId, sizeof(logBuffer), nullptr, logBuffer);
            printf("%s\n", logBuffer);
            glDeleteShader(shaderId);
        }
        else
        {
            this->ShaderId = shaderId;
        }
    });
    return true;
}

void CShaderObject::Cleanup()
{
    Device->WaitForTask(ShaderIdReady);
    Source.clear();
}

CShaderProgram::CShaderProgram(std::shared_ptr<CShaderObject> vs, std::shared_ptr<CShaderObject> ps)
    : VS(std::move(vs))
    , PS(std::move(ps))
{
}

bool CShaderProgram::Link()
{
    if (!VS)
        return false;
    if (!VS->Device)
        return false;
    VS->Device->QueueTask(
        [this]() {
            GLint isLinked = 0;
            GLuint programId = glCreateProgram();
            glAttachShader(programId, VS->ShaderId);
            glAttachShader(programId, PS->ShaderId);
            glLinkProgram(programId);
            glGetProgramiv(programId, GL_LINK_STATUS, &isLinked);
            if (isLinked == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);
                std::vector<GLchar> infoLog(maxLength);
                glGetProgramInfoLog(programId, maxLength, &maxLength, &infoLog[0]);
                glDeleteProgram(programId);
                printf("%s\n", infoLog.data());
            }
            else
            {
                // Gather reflection data, no locking is required since bWait=true
                GLint activeUniformNum;
                std::unordered_map<std::string, int> uniformCache;
                glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &activeUniformNum);
                for (GLint i = 0; i < activeUniformNum; i++)
                {
                    const int bufLen = 64;
                    char name[bufLen];
                    GLsizei nameLen;
                    GLint size;
                    GLenum type;
                    glGetActiveUniform(programId, i, bufLen, &nameLen, &size, &type, name);
                    assert(bufLen >= nameLen);
                    printf("GL Program %u@%d: %s size=%d type=%d\n", programId, i, name, size, type);
                    uniformCache[name] = i;
                }
                // Also support querying uniform buffer bindings by name
                GLint activeUniformBlocks;
                glGetProgramiv(programId, GL_ACTIVE_UNIFORM_BLOCKS, &activeUniformBlocks);
                std::unordered_map<std::string, int> uniformBufferCache;
                for (GLint i = 0; i < activeUniformBlocks; i++)
                {
                    const int bufSize = 64;
                    char name[bufSize];
                    GLsizei nameLen;
                    glGetActiveUniformBlockName(programId, i, bufSize, &nameLen, name);
                    printf("GL Program %u: uniform block %s at %d\n", programId, name, i);
                    uniformBufferCache[name] = i;
                    // HACK: always use uniform block index + 1 as binding slot
                    // See CommandBuffer for how it uses this assumption
                    glUniformBlockBinding(programId, i, i + 1);
                }
                this->ProgramId = programId;
                this->UniformLocationCache = std::move(uniformCache);
                this->UniformBufferLocationCache = std::move(uniformBufferCache);
            }
        },
        false, true);
    return ProgramId != 0;
}

GLint CShaderProgram::GetUniformLocation(const std::string& name)
{
    {
        auto iter = UniformLocationCache.find(name);
        if (iter != UniformLocationCache.end())
            return iter->second;
    }
    {
        auto iter = UniformBufferLocationCache.find(name);
        if (iter != UniformBufferLocationCache.end())
            return iter->second;
    }
    return -1;
}
