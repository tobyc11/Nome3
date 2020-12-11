#pragma once
#include <Shaders.h>

class CPipelineLibrary
{
public:
    static std::shared_ptr<CShaderObject> FindShader(CGLThread* device, EShaderStage type, const std::string& name);
};
