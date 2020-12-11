#pragma once
#include "CGLThread.h"
#include <memory>
#include <string>
#include <utility>

enum class EShaderStage : int
{
    Vertex,
    Fragment
};

int EShaderStageToGL(EShaderStage input);

class CShaderObject
{
public:
    CShaderObject(CGLThread* device, EShaderStage type, std::string source);

    ~CShaderObject();

    [[nodiscard]] bool IsValid() const;

private:
    bool Compile();

    void Cleanup();

    friend class CShaderProgram;

    CGLThread* Device;
    EShaderStage Type; // GL_???_SHADER
    std::string Source;

    time_marker_t ShaderIdReady = 0;
    unsigned int ShaderId {};
};

class CShaderProgram
{
public:
    CShaderProgram(std::shared_ptr<CShaderObject> vs, std::shared_ptr<CShaderObject> ps);

    bool Link();

    [[nodiscard]] unsigned int GetProgramId() const { return ProgramId; }

    int GetUniformLocation(const std::string& name);

private:
    std::shared_ptr<const CShaderObject> VS, PS;
    unsigned int ProgramId {};
    std::unordered_map<std::string, int> UniformLocationCache;
    std::unordered_map<std::string, int> UniformBufferLocationCache;
};
