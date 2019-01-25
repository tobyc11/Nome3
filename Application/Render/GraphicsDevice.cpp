#include "GraphicsDevice.h"

#include <StringPrintf.h>
#include <StringUtils.h>

namespace Nome
{

CGraphicsDevice::CGraphicsDevice()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		DriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, DriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &D3dDevice, &FeatureLevel, &ImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, DriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &D3dDevice, &FeatureLevel, &ImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		throw CGraphicsException(tc::StringPrintf("HRESULT = %d", hr));

	RetrieveDesc();
}

CGraphicsDevice::CGraphicsDevice(ID3D11Device* device)
{
	D3dDevice = device;
	D3dDevice->AddRef();
    D3dDevice->GetImmediateContext(&ImmediateContext);

	RetrieveDesc();
}

CGraphicsDevice::~CGraphicsDevice()
{
	if (ImmediateContext) ImmediateContext->ClearState();

	if (ImmediateContext) ImmediateContext->Release();
	if (D3dDevice) D3dDevice->Release();
}

void CGraphicsDevice::RetrieveDesc()
{
	HRESULT hr = S_OK;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = D3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));

				DXGI_ADAPTER_DESC desc;
				hr = adapter->GetDesc(&desc);
				if (SUCCEEDED(hr))
				{
					Description = tc::FStringUtils::UTF16to8(desc.Description);
				}

				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		throw CGraphicsException(tc::StringPrintf("HRESULT = %d", hr));
	dxgiFactory->Release();
}

} // namespace Nome
