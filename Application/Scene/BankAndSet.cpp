#include "BankAndSet.h"

namespace Nome::Scene
{

CSlider::CSlider(CCommandHandle handle, float min, float max, float step)
	: Handle(handle), Min(min), Max(max), Step(step)
{
}

void CSlider::DrawImGui()
{
	//TODO
}

void CBankAndSet::AddSlider(const std::string& name, CCommandHandle handle, float value, float min, float max, float step)
{
	if (GetSlider(name))
	{
		throw std::runtime_error("Slider already exists");
	}
	auto* slider = new CSlider(handle, min, max, step);
	slider->SetNumber(value);
	Sliders.insert({ name, slider });
}

CSlider* CBankAndSet::GetSlider(const std::string& name)
{
	auto iter = Sliders.find(name);
	if (iter != Sliders.end())
		return iter->second;
	return nullptr;
}

}
