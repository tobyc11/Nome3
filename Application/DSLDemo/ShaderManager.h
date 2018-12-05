#pragma once

#include "GraphicsDevice.h"
#include "ConvertToHLSLType.h"

#include <string>
#include <vector>

namespace Nome
{

struct ConstantBufferMeta
{
	ConstantBufferMeta(const std::string& name, unsigned int slot) : Name(name), Slot(slot)
	{
	}

	std::string Name;
	unsigned int Slot;
	std::vector<std::string> MemberTypes;
	std::vector<std::string> MemberNames;

	std::string ToString() const;
};

struct ConstantBufferAddMember
{
	ConstantBufferAddMember(ConstantBufferMeta& meta, const std::string& type, const std::string& name)
	{
		meta.MemberTypes.push_back(type);
		meta.MemberNames.push_back(name);
	}
};

#define DEFINE_CBUFFER(Name, Slot) struct Name { ConstantBufferMeta _Meta{ #Name, Slot };
#define DEFINE_CBUFFER_MEMBER(Type, Name) \
Type Name;\
ConstantBufferAddMember _Adder##Name{ _Meta, TConvertToHLSLType<Type>::Get(), #Name };
#define END_CBUFFER() };

DEFINE_CBUFFER(StageParams, 0)
DEFINE_CBUFFER_MEMBER(Matrix4, MatMul)
END_CBUFFER()

class CShaderManager
{
public:
	CShaderManager(CGraphicsDevice* gdevice) : GD(gdevice)
	{
	}

	void CompileFromFile(const std::string& fileName, const D3D_SHADER_MACRO* pDefines,
		LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1, UINT Flags2, ID3DBlob** ppCode);

private:
	CGraphicsDevice* GD;
};

} // namespace Nome
