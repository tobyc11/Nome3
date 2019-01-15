#pragma once
#include <Flow/Arithmetics.h>
#include <Parsing/ASTUtils.h>
#include <map>

namespace Nome::Scene
{

class CSlider : public Flow::CFloatNumber
{
public:
	CSlider(CCommandHandle handle, float min, float max, float step);

	void DrawImGui();

private:
	CCommandHandle Handle;
	float Min;
	float Max;
	float Step;
};

//Manages all the sliders
class CBankAndSet
{
public:
	void AddSlider(const std::string& name, CCommandHandle handle, float value, float min, float max, float step);
	CSlider* GetSlider(const std::string& name);

	void DrawImGui();

private:
	std::map<std::string, tc::TAutoPtr<CSlider>> Sliders;
};

}
