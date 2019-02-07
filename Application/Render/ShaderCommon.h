#pragma once
#include "IncludeD3D11.h"
#include <d3dcompiler.h>
#include <string>
#include <fstream>
#include <iostream>

namespace Nome
{

class CShaderCommon
{
protected:
    bool CompileFile(const std::string& fileName, const char* entry, const char* target)
    {
        //Read the shader template from disk
        std::ifstream ifs(fileName);
        std::string str;

        ifs.seekg(0, std::ios::end);
        str.reserve(ifs.tellg());
        ifs.seekg(0, std::ios::beg);

        str.assign((std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>());

        //Compile vertex shader
        ID3DBlob* codeBlob;
        ID3DBlob* errorBlob;
        D3DCompile(str.c_str(), str.size(), fileName.c_str(), nullptr, nullptr, entry, target, 0, 0, &codeBlob, &errorBlob);
        if (errorBlob)
        {
            std::cout << "[" << target << "] Shader compilation error:" << std::endl;
            std::cout << reinterpret_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
            errorBlob->Release();
            return false;
        }

        if (target[0] == 'v')
            VSBytecode = codeBlob;
        else if (target[0] == 'g')
            GSBytecode = codeBlob;
        else if (target[0] == 'p')
            PSBytecode = codeBlob;
        codeBlob->Release();
        return true;
    }

    ComPtr<ID3DBlob> VSBytecode;
    ComPtr<ID3D11VertexShader> VS;
    ComPtr<ID3DBlob> GSBytecode;
    ComPtr<ID3D11GeometryShader> GS;
    ComPtr<ID3DBlob> PSBytecode;
    ComPtr<ID3D11PixelShader> PS;
};

}
