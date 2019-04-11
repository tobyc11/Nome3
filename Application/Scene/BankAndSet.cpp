#include "BankAndSet.h"
#include <StringPrintf.h>
#include <imgui.h>

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

void CSlider::DrawImGui(const std::string& name)
{
    ImGui::SliderFloat(tc::StringPrintf("%s (%.3f - %.3f)", name.c_str(), Min, Max).c_str(),
                       &GuiValue, Min, Max);
    if (GuiValue != GetNumber())
    {
        SetNumber(GuiValue);
    }
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

void CBankAndSet::DrawImGui()
{
    ImGui::Begin("Slider Banks");
    ImGui::Text("Control-Click to manually enter numbers.");
    std::string lastBankName;
    for (const auto& pair : Sliders)
    {
        size_t dotOffset = pair.first.find('.');
        std::string bankName = pair.first.substr(0, dotOffset);
        pair.second->DrawImGui(pair.first);
    }
    ImGui::End();
}

void CBankAndSet::WriteSliderValues()
{
    for (const auto& pair : Sliders)
    {
        pair.second->WriteValue();
    }
}

}
