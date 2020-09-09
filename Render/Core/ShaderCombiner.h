#pragma once

#include <DrawTemplate.h>

#include <string>
#include <set>

namespace Nome::Render
{

class CShaderCombiner
{
public:
    static void ClearShaderCache();

    void SetVertexShader(const std::string& type);
    void SetSurfaceInteraction(const std::string& type);
    void SetIntegrator(const std::string& type);
    void SetMaterial(const std::string& type);
    void AddCompileDefinition(const std::string& def);

    void BindToDrawTemplate(RHI::CDrawTemplate& drawTemplate);

private:
    std::string VertexShaderType;
    std::string SurfaceInteractionType;
    std::string IntegratorType;
    std::string MaterialType;
    std::set<std::string> CompileDefs;
};

} /* namespace Nome::Render */
