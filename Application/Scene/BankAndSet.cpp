#include "BankAndSet.h"
#include <StringPrintf.h>

namespace Nome::Scene
{

CSlider::CSlider(CCommandHandle handle, float value, float min, float max, float step)
    : Handle(handle)
    , GuiValue(value)
    , Min(min)
    , Max(max)
    , Step(step)
{
}

void CSlider::WriteValue()
{
    CCommandRewriter rewriter { Handle };
    rewriter.ReplaceArg(0, std::to_string(GuiValue));
}

void CBankAndSet::AddSlider(const std::string& name, CCommandHandle handle, float value, float min,
                            float max, float step)
{
    if (GetSlider(name))
    {
        throw std::runtime_error("Slider already exists");
    }
    auto* slider = new CSlider(handle, value, min, max, step);
    slider->SetNumber(value);
    Sliders.insert({ name, slider });
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

void CBankAndSet::WriteSliderValues()
{
    for (const auto& pair : Sliders)
    {
        pair.second->WriteValue();
    }
}

}
