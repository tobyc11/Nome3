#include "BankAndSet.h"

namespace Nome::Scene
{

CSlider::CSlider(AST::ACommand* cmd, float value, float min, float max, float step)
    : Cmd(cmd)
    , Min(min)
    , Max(max)
    , Step(step)
{
    this->SetNumber(value);
}

void CSlider::SetValue(float value)
{
    SetNumber(value);
}

CBankAndSet::~CBankAndSet()
{
    int sliderIndex = 0;
    for (const auto& nameSlider : SliderList)
        for (auto* observer : Observers)
            observer->OnSliderRemoving(*nameSlider, SliderNameList.at(sliderIndex++));
}

void CBankAndSet::AddSlider(const std::string& name, AST::ACommand* cmd, float value, float min, float max, float step)
{
    if (GetSlider(name))
        throw std::runtime_error("Slider already exists");

    auto* slider = new CSlider(cmd, value, min, max, step);
    Sliders.insert({ name, slider });

    for (auto* observer : Observers)
        observer->OnSliderAdded(*slider, name);
}

void CBankAndSet::AddToSliderList(const std::string name)
{
    SliderNameList.push_back(name);
    SliderList.push_back(Sliders.at(name));
}

CSlider* CBankAndSet::GetSlider(const std::string& name)
{
    std::map<std::string, tc::TAutoPtr<CSlider>>::iterator iter;
    if (name.size() > 1 && name[0] == '$')
        iter = Sliders.find(name.substr(1));
    else
        iter = Sliders.find(name);

    if (iter != Sliders.end())
        return iter->second;
    return nullptr;
}

void CBankAndSet::AddObserver(ISliderObserver* observer)
{
    int sliderIndex = 0;
    for (const auto& nameSlider : SliderList)
        observer->OnSliderAdded(*nameSlider, SliderNameList.at(sliderIndex++));
    Observers.insert(observer);
}

}
