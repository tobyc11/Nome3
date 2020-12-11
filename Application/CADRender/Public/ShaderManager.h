#pragma once
#include "RenderContext.h"
#include "Shaders.h"
#include <unordered_map>

class CShaderManager : public CRenderContextChild
{
public:
    std::shared_ptr<CShaderProgram> GetShaderProgramByNames(const std::string& vs, const std::string& ps);

private:
    std::shared_ptr<CShaderObject> FindOrCreateShaderObject(EShaderStage stage, const std::string& name);

    std::unordered_map<std::string, std::shared_ptr<CShaderObject>> ShaderCache;
    std::unordered_map<std::string, std::shared_ptr<CShaderProgram>> ProgramCache;
};
