#include "ShaderManager.h"
#include "PipelineLibrary.h"

std::shared_ptr<CShaderProgram> CShaderManager::GetShaderProgramByNames(const std::string& vs, const std::string& ps)
{
    auto programIdent = vs + "+" + ps;
    auto programIter = ProgramCache.find(programIdent);
    if (programIter != ProgramCache.end()) {
        return programIter->second;
    }
    std::shared_ptr<CShaderObject> vso, pso;
    vso = FindOrCreateShaderObject(EShaderStage::Vertex, vs);
    pso = FindOrCreateShaderObject(EShaderStage::Fragment, ps);
    if (!vso || !pso) {
        printf("Failed to create program %s %s\n", vs.c_str(), ps.c_str());
        return nullptr;
    }
    auto program = std::make_shared<CShaderProgram>(vso, pso);
    printf("Linking program vs:%s fs:%s\n",vs.c_str(), ps.c_str());
    program->Link();
    ProgramCache[programIdent] = program;
    return program;
}

std::shared_ptr<CShaderObject> CShaderManager::FindOrCreateShaderObject(EShaderStage stage, const std::string& name)
{
    auto iter = ShaderCache.find(name);
    if (iter != ShaderCache.end())
    {
        return iter->second;
    }
    else
    {
        auto obj = CPipelineLibrary::FindShader(&ParentCtx->GetDevice(), stage, name);
        if (!obj)
            return nullptr;
        ShaderCache[name] = obj;
        return obj;
    }
}
