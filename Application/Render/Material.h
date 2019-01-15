#pragma once
#include <Vector3.h>

namespace Nome
{

using tc::Vector3;

//Minimally featured. Extend later when we have time.
class CMaterial
{
public:
	Vector3 GetColor() const { return Color; }
	void SetColor(const Vector3& value) { Color = value; }

private:
	Vector3 Color;
};

}