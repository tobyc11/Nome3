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
	std::vector<EDataType> MemberTypes;
	std::vector<std::string> MemberNames;

	std::string ToString() const;
	size_t GetSize() const;
	ID3D11Buffer* CreateConstantBuffer(ID3D11Device* dev) const;
};

struct ConstantBufferAddMember
{
	ConstantBufferAddMember(ConstantBufferMeta& meta, EDataType dataType, const std::string& name)
	{
		meta.MemberTypes.push_back(dataType);
		meta.MemberNames.push_back(name);
	}
};

#define DEFINE_CBUFFER(Name, Slot) struct Name { //static ConstantBufferMeta _Meta{ #Name, Slot };
#define DEFINE_CBUFFER_MEMBER(Type, Name) \
Type Name;\
//static ConstantBufferAddMember _Adder##Name{ _Meta, ToDataType<Type>::Result, #Name };
#define END_CBUFFER() };

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
