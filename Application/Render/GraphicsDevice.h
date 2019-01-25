#pragma once

#include <d3d11_1.h>

#include <stdexcept>
#include <string>

namespace Nome
{

class CShaderManager;

class CGraphicsException : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

class CGraphicsDevice
{
public:
	///Create a dvice from scratch (OS)
	CGraphicsDevice();
	///Create from existing device
	CGraphicsDevice(ID3D11Device* device);

	~CGraphicsDevice();

	ID3D11Device* GetDevice() const
	{
		return D3dDevice;
    }
	
	ID3D11DeviceContext* GetImmediateContext() const
	{
		return ImmediateContext;
	}

	CShaderManager* GetShaderManager() const
	{
		return ShaderManager;
	}

	const std::string& GetDescription() const { return Description; }

private:
	void RetrieveDesc();

	D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* D3dDevice;
	ID3D11DeviceContext* ImmediateContext;

	CShaderManager* ShaderManager;

	std::string Description;
};

} // namespace Nome
