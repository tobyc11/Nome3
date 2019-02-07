#pragma once
#include <functional>
#include <map>
#include <type_traits>

namespace tc
{

template <typename Signature>
class FSignal
{
public:
    FSignal() = default;

    //Disallow copy
    FSignal(const FSignal&) = delete;
    FSignal& operator=(const FSignal&) = delete;

    //Disallow move
    FSignal(FSignal&&) = delete;
    FSignal& operator=(FSignal&&) = delete;

    unsigned int Connect(std::function<Signature> slot)
    {
        Slots.emplace(CurrentId, std::move(slot));
        return CurrentId++;
    }

    void Disconnect(unsigned int id)
    {
        Slots.erase(id);
    }

    void DisconnectAll()
    {
        Slots.clear();
    }

    template <typename... Args>
    void operator()(const Args&&... args)
    {
        for (auto pair : Slots)
        {
            pair.second(std::forward<Args>(args)...);
        }
    }
        
private:
    std::map<unsigned int, std::function<Signature>> Slots;
    unsigned int CurrentId = 0;
};

}
