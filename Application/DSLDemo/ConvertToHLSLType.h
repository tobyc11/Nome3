#pragma once

#include "DataTypeHelper.h"

#include <dxgiformat.h>

namespace Nome
{

template <typename T>
struct TConvertToHLSLType
{
	static const char* Get()
	{
		return "invalid-type";
	}
};

template <>
struct TConvertToHLSLType<float>
{
	static const char* Get() { return "float"; }
};

template <>
struct TConvertToHLSLType<Vector2>
{
	static const char* Get() { return "float2"; }
};

template <>
struct TConvertToHLSLType<Vector3>
{
	static const char* Get() { return "float3"; }
};

template <>
struct TConvertToHLSLType<Vector4>
{
	static const char* Get() { return "float4"; }
};

template <>
struct TConvertToHLSLType<Matrix3>
{
	static const char* Get() { return "float3x3"; }
};

template <>
struct TConvertToHLSLType<Matrix4>
{
	static const char* Get() { return "float4x4"; }
};

inline std::string ConvertToHLSLType(EDataType dataType)
{
	switch (dataType)
	{
	case EDataType::Float:
		return "float";
	case EDataType::Float3:
		return "float3";
	case EDataType::Float4:
		return "float4";
	case EDataType::Float3x3:
		return "float3x3";
	case EDataType::Float4x4:
		return "float4x4";
	case EDataType::Invalid:
	default:
		return "invalid-type";
	}
}

inline DXGI_FORMAT ConvertToDXGIFormat(EDataType dataType)
{
	switch (dataType)
	{
	case EDataType::Invalid:
		return DXGI_FORMAT_UNKNOWN;
	case EDataType::Float:
		return DXGI_FORMAT_R32_FLOAT;
	case EDataType::Float3:
		return DXGI_FORMAT_R32G32B32_FLOAT;
	case EDataType::Float4:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case EDataType::Float3x3:
		return DXGI_FORMAT_UNKNOWN;
	case EDataType::Float4x4:
		return DXGI_FORMAT_UNKNOWN;
	default:
		break;
	}
	return DXGI_FORMAT_UNKNOWN;
}

} // namespace Nome
