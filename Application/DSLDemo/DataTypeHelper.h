#pragma once

#include <Vector3.h>
#include <Matrix4.h>

namespace Nome
{

using tc::Vector2;
using tc::Vector3;
using tc::Vector4;

using tc::Matrix3;
using tc::Matrix4;

enum class EDataType
{
	Invalid,
	Float,
	Float3,
	Float4,
	Float3x3,
	Float4x4,
};

template <typename T>
class ToDataType
{
public:
    static const EDataType Result = EDataType::Invalid;
};

template <> class ToDataType<float> { public: static const EDataType Result = EDataType::Float; };
template <> class ToDataType<Vector3> { public: static const EDataType Result = EDataType::Float3; };
template <> class ToDataType<Vector4> { public: static const EDataType Result = EDataType::Float4; };
template <> class ToDataType<Matrix3> { public: static const EDataType Result = EDataType::Float3x3; };
template <> class ToDataType<Matrix4> { public: static const EDataType Result = EDataType::Float4x4; };

template <EDataType dt>
class CTypeFromEnum
{
public:
	typedef void Type;
};

template <>
class CTypeFromEnum<EDataType::Float>
{
public:
	typedef float Type;
};

template <>
class CTypeFromEnum<EDataType::Float3>
{
public:
	typedef Vector3 Type;
};

template <>
class CTypeFromEnum<EDataType::Float4>
{
public:
	typedef Vector4 Type;
};

template <>
class CTypeFromEnum<EDataType::Float3x3>
{
public:
	typedef Matrix3 Type;
};

template <>
class CTypeFromEnum<EDataType::Float4x4>
{
public:
	typedef Matrix4 Type;
};

inline size_t DataTypeToSize(EDataType dataType)
{
	switch (dataType)
	{
	case EDataType::Float:
		return sizeof(CTypeFromEnum<EDataType::Float>::Type);
	case EDataType::Float3:
		return sizeof(CTypeFromEnum<EDataType::Float3>::Type);
	case EDataType::Float4:
		return sizeof(CTypeFromEnum<EDataType::Float4>::Type);
	case EDataType::Float3x3:
		return sizeof(CTypeFromEnum<EDataType::Float3x3>::Type);
	case EDataType::Float4x4:
		return sizeof(CTypeFromEnum<EDataType::Float4x4>::Type);
	case EDataType::Invalid:
	default:
		return 0;
	}
}

} /* namespace Nome */
