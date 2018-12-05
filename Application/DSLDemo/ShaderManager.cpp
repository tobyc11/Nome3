#include "ShaderManager.h"
#include "ResourceManager.h"

#include <StringUtils.h>
#include <StringPrintf.h>

#include <d3dcompiler.h>

namespace Nome
{

std::string ConstantBufferMeta::ToString() const
{
	std::string result;
	result += tc::StringPrintf("cbuffer %s : register(b%d)\n", Name.c_str(), Slot);
	result += "{\n";
	for (size_t i = 0; i < MemberNames.size(); i++)
	{
		result += tc::StringPrintf("%s %s;\n", MemberTypes[i].c_str(), MemberNames[i].c_str());
	}
	result += "}\n";
	return result;
}

void CShaderManager::CompileFromFile(const std::string& fileName, const D3D_SHADER_MACRO* pDefines,
	LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode)
{
	std::string filePath = CResourceManager::LocateFile(fileName);
	auto filePathW = tc::FStringUtils::UTF8to16(filePath.c_str());

#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	Flags1 |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob = nullptr;

#if D3D_COMPILER_VERSION >= 46
	D3DCompileFromFile(filePathW.c_str(), pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		pEntrypoint, pTarget, Flags1, Flags2, ppCode, &pErrorBlob);
#else
	throw "Unimplemented";
#endif

#pragma warning( suppress : 6102 )
	if (pErrorBlob)
	{
		OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		pErrorBlob->Release();
	}
}

} // namespace Nome
