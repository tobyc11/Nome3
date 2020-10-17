#include "BankAndSet.h"

namespace Nome::Scene
{

CSlider::CSlider(AST::ACommand* cmd, float value, float min, float max, float step, std::string animfunc)
    : Cmd(cmd)
    , Min(min)
    , Max(max)
    , Step(step)
    , AnimFunc(animfunc)
{
    this->SetNumber(value);
}

void CSlider::SetValue(float value)
{
    SetNumber(value);
}

void CSlider::SetAnimMax(float x) {
    AnimMax = x;
}

void CSlider::SetAnimMin(float x) {
    AnimMin = x;
}

CBankAndSet::~CBankAndSet()
{
    for (const auto& nameSlider : Sliders)
        for (auto* observer : Observers)
            observer->OnSliderRemoving(*nameSlider.second, nameSlider.first);
}

void CBankAndSet::AddSlider(const std::string& name, AST::ACommand* cmd, float value, float min,
                            float max, float step, std::string animfunc)
{
    if (GetSlider(name))
    {
        throw std::runtime_error("Slider already exists");
    }
    auto* slider = new CSlider(cmd, value, min, max, step, animfunc);
    Sliders.insert({ name, slider });

    for (auto* observer : Observers)
        observer->OnSliderAdded(*slider, name);
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
    for (const auto& nameSlider : Sliders)
        observer->OnSliderAdded(*nameSlider.second, nameSlider.first);
    Observers.insert(observer);
}

}
